#pragma once
#include <ShaderDebugger/Translator.h>
#include <glm/glm.hpp>

extern "C" {
	#include <BlueVM.h>
}

namespace sd
{
	class ShaderDebugger
	{
	public:
		ShaderDebugger();
		~ShaderDebugger();

		template<typename CodeTranslator>
		bool SetSource(sd::ShaderType stage, const std::string& src, const std::string& entry, bv_library* library)
		{
			if (m_transl != nullptr)
				delete m_transl;
			
			m_transl = new CodeTranslator();

			m_entry = entry;
			m_library = library;

			CodeTranslator tr;
			bool done = m_transl->Parse(stage, src, entry);
			std::vector<uint8_t> bytecode = m_transl->GetBytecode();
			
			if (done && bytecode.size() > 0) {
				m_prog = bv_program_create(bytecode.data());
				if (m_prog == nullptr)
					return false; // invalid bytecode
					
				bv_function* entryPtr = bv_program_get_function(m_prog, entry.c_str());
				m_stepper = bv_function_stepper_create(m_prog, entryPtr, NULL, NULL);
				
				if (m_library != nullptr)
					bv_program_add_library(m_prog, library);
			} else return false;

			return true;
		}

		inline bv_variable Execute() { return Execute(m_entry); }
		bv_variable Execute(const std::string& func); // TODO: arguments


		std::string GetCurrentFunction();
		std::vector<std::string> GetFunctionStack();
		std::vector<std::string> GetCurrentFunctionLocals();
		bv_variable* GetLocalValue(const std::string& varname);
		int GetCurrentLine() { return m_prog->current_line; }
		bool Step();

		// for more complex types, we need to provide classType (for example, vec3 is for GLSL but float3 is for HLSL)
		// this makes ShaderDebugger work without needing to know which shader language it uses
		void SetValue(const std::string& varName, float value);
		void SetValue(const std::string& varName, const std::string& classType, glm::vec3 val);

		bv_variable* GetValue(const std::string& gvarname);

	private:
		Translator* m_transl;
		std::string m_entry;
		bv_library* m_library;
		bv_program* m_prog;
		bv_function_stepper* m_stepper;
	};
}