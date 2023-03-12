#include "hlslReflection.h"
#ifdef DX12_BACKEND
#include <sstream>
#include <filesystem>
#include <fstream>

#include "utilsReflectDx12.h"
using namespace KUMA;
using namespace KUMA::RENDER;
SimpleHLSLReflection::SimpleHLSLReflection(const std::string& path, const std::string& entryPoint, ShaderType type) {
	m_folder = std::filesystem::path{ path }.parent_path().string();

	extractInformationFromShaderFile(path, entryPoint, type);
}

void SimpleHLSLReflection::readFileLines(const std::string& path, std::list<std::string>& curShaderLines) const {
	std::ifstream file(path);
	std::string s;
	while (getline(file, s)) {
		s = UTILS::trim(s);
		if (s.size() >= 2) {
			if (s[0] == s[1] && s[1] == '/') {
				continue;
			}
		}
		curShaderLines.push_back(s);
	}
}

void SimpleHLSLReflection::extractInformationFromShaderFile(const std::string& path, const std::string& entryPoint, ShaderType type) {
	std::list<std::string> curShaderLines;
	readFileLines(path, curShaderLines);


	auto splitStringToLexems = [](const std::string& s) {
		std::istringstream iss(s);
		std::vector<std::string> _results(std::istream_iterator<std::string>{iss},
			std::istream_iterator<std::string>());

		std::vector<std::string> results;
		for (auto& s : _results) {
			std::string newLexem;
			for (auto ch : s) {
				if (std::isalnum(ch) || ch == '_') {
					newLexem += ch;
				}
				else {
					if (!newLexem.empty()) {
						results.push_back(newLexem);
						newLexem = "";
					}
					results.push_back(std::string(1, ch));
				}
			}
			if (!newLexem.empty()) {
				results.push_back(newLexem);
			}
		}
		return results;
	};
	auto extractStructName = [](const std::vector<std::string>& lexems) {
		if (lexems.size() <= 1) {
			throw;
		}
		return lexems[1];
	};
	auto getArrSize = [this](const std::vector<std::string>& lexems) {
		int arrSize = 1;
		if (lexems[2] == "[") {
			auto arrSizeStr = lexems[3];
			if (UTILS::isNumber(arrSizeStr)) {
				arrSize = std::stoi(arrSizeStr);
			}
			else if (defines.count(arrSizeStr)) {
				arrSizeStr = defines[arrSizeStr];
				if (UTILS::isNumber(arrSizeStr)) {
					arrSize = std::stoi(arrSizeStr);
				}
				else {
					throw;
				}
			}
			else {
				throw;
			}
		}
		return arrSize;
	};
	auto extractStructMember = [getArrSize, this](const std::vector<std::string>& lexems) {
		int arrSize = getArrSize(lexems);
		return StructMemberDescriptor{ lexems[1], getVarType(lexems[0]), arrSize,
			lexems[lexems.size() - 3] == ":" ? lexems[lexems.size() - 2] : "" };
	};
	auto extractUniformMember = [getArrSize, this](const std::vector<std::string>& lexems) {
		int arrSize = getArrSize(lexems);
		return UniformMemberDescriptor{ lexems[1], getVarType(lexems[0]), arrSize };
	};

	auto getNumSuffix = [](const std::string& s) {
		std::string numStr;
		int i = s.size() - 1;
		while (i >= 0 && std::isdigit(s[i])) {
			numStr = s[i] + numStr;
			i--;
		}
		return numStr;
	};

	//std::set<std::string> includes;
	for (auto stri = curShaderLines.begin(); stri != curShaderLines.end();) {
		auto& s = *stri;
		if (UTILS::isConsistInString(s, "#include")) {
			extractInformationFromShaderFile(m_folder + "/" + UTILS::extractFromString(s, "\"", "\""), entryPoint, type);
		}
		else if (UTILS::isConsistInString(s, "#define")) {
			auto lexems = splitStringToLexems(s);
			defines[lexems[2]] = lexems[3];
		}
		else if (UTILS::findPos(s, "struct ") == 0) {
			StructDescriptor structDescriptor{};
			structDescriptor.name = extractStructName(splitStringToLexems(s));
			if (structsDescriptors.count(structDescriptor.name)) {
				continue;
			}
			stri++;
			s = *stri;

			while (true) {
				auto lexems = splitStringToLexems(s);
				if (lexems.size() > 1) {
					if (lexems[lexems.size() - 1] == ";" && lexems[lexems.size() - 2] == "}") {
						break;
					}
					structDescriptor.members.push_back(extractStructMember(lexems));
				}
				stri++;
				s = *stri;
			}
			stri++;
			s = *stri;
			structsDescriptors[structDescriptor.name] = structDescriptor;
		}
		else if (UTILS::isConsistInString(s, entryPoint)) {
			auto lexems = splitStringToLexems(s);
			if (type == ShaderType::VERTEX) {
				if (lexems.size() < 5) {
					throw;
				}
				vertexDescriptor = structsDescriptors.count(lexems[3]) ?
					structsDescriptors[lexems[3]] :
					StructDescriptor{ "", {{"", getVarType(lexems[3]), 1, ""}} };
			}
			else if (type == ShaderType::FRAGMENT) {
				colorDescriptor = structsDescriptors.count(lexems[0]) ?
					structsDescriptors[lexems[0]] :
					StructDescriptor{ "", {{"", getVarType(lexems[0]), 1, ""}} };
			}

		}
		else if (UTILS::isConsistInString(s, "register")) { //uniforms
			auto lexems = splitStringToLexems(s);
			if (UTILS::isConsistInString(s, "Texture2D") || UTILS::isConsistInString(s, "TextureCube") || UTILS::isConsistInString(s, "SamplerState")) {
				auto descriptor = UniformDescriptor{};
				descriptor.name = lexems[1];
				descriptor.type = getUniformType(lexems[0]);
				descriptor.bind = std::stoi(getNumSuffix(lexems[5]));
				uniformsDescriptors[descriptor.name] = descriptor;
			}
			else if (UTILS::isConsistInString(s, "StructuredBuffer")) {
				auto descriptor = UniformDescriptor{};
				descriptor.name = lexems[4];
				descriptor.type = getUniformType(lexems[0]);
				descriptor.subtypeType = getVarType(lexems[2]);
				descriptor.subtypeName = lexems[2];
				descriptor.bind = std::stoi(getNumSuffix(lexems[8]));
				descriptor.space = std::stoi(getNumSuffix(lexems[10]));
				uniformsDescriptors[descriptor.name] = descriptor;
			}
			else if (UTILS::isConsistInString(s, "cbuffer")) {
				auto descriptor = UniformDescriptor{};
				descriptor.name = lexems[1];
				descriptor.type = getUniformType(lexems[0]);
				descriptor.bind = std::stoi(getNumSuffix(lexems[5]));

				stri++;
				s = *stri;
				while (true) {
					auto lexems = splitStringToLexems(s);
					if (lexems.size() > 1) {
						if (lexems[lexems.size() - 1] == ";" && lexems[lexems.size() - 2] == "}") {
							break;
						}
						descriptor.members.push_back(extractUniformMember(lexems));
					}
					stri++;
					s = *stri;
				}

				uniformsDescriptors[descriptor.name] = descriptor;
			}

		}
		stri++;
	}
}
#endif