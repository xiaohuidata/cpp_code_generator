#ifndef CODE_GENERATOR_CONFIG_PARSER_H
#define CODE_GENERATOR_CONFIG_PARSER_H

#include "cpp_generator.h"
#include <boost/json.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace code_generator {

namespace json = boost::json;

// 配置数据结构
struct CodeGenConfig {
	struct ClassConfig {
		std::string name;
		std::vector<std::string> base_classes;
		std::vector<std::string> templates;
		std::map<std::string, std::string> metadata;

		static ClassConfig FromJson(const json::value& json);
		json::value ToJson() const;
	};

	struct FunctionConfig {
		std::string name;
		std::string return_type;
		std::vector<std::pair<std::string, std::string>> parameters; // type, name
		std::string body;
		std::string access = "public";
		bool is_virtual = false;
		bool is_pure_virtual = false;
		bool is_const = false;
		bool is_static = false;
		std::vector<std::string> templates;

		static FunctionConfig FromJson(const json::value& json);
		json::value ToJson() const;
	};

	struct MemberConfig {
		std::string name;
		std::string type;
		std::string initializer;
		std::string access = "private";
		std::string comment;

		static MemberConfig FromJson(const json::value& json);
		json::value ToJson() const;
	};

	struct FileConfig {
		std::string filename;
		std::string type; // "header" or "source"
		std::vector<std::string> includes;
		std::vector<std::string> namespaces;
		std::vector<ClassConfig> classes;
		std::vector<FunctionConfig> functions;
		std::vector<MemberConfig> globals;
		std::map<std::string, std::string> templates;
		std::vector<std::string> copy_files;
		std::vector<std::string> insert_snippets;

		static FileConfig FromJson(const json::value& json);
		json::value ToJson() const;
	};

	struct ProjectConfig {
		std::string name;
		std::string version;
		std::string output_dir;
		std::vector<FileConfig> files;
		std::map<std::string, std::string> variables;
		std::vector<std::string> common_includes;
		std::map<std::string, std::string> code_templates;

		static ProjectConfig FromJson(const json::value& json);
		json::value ToJson() const;
	};

	static void LoadVariables(const json::value& json, std::map<std::string, std::string>& variables);
};

// 配置解析器
class ConfigParser {
public:
	ConfigParser();

	// 从文件加载配置
	bool LoadFromFile(const boost::filesystem::path& filename);

	// 从字符串加载配置
	bool LoadFromString(const std::string& json_str);

	// 从JSON值加载配置
	bool LoadFromJson(const json::value& json);

	// 获取项目配置
	const CodeGenConfig::ProjectConfig& GetProjectConfig() const { return project_config_; }

	// 变量替换
	std::string ReplaceVariables(const std::string& text) const;
	void ReplaceBufferByVariables(std::string& strjson, std::map<std::string, std::string>& variables);

	// 获取代码模板
	std::string GetTemplate(const std::string& name) const;

	// 应用模板并替换变量
	std::string ApplyTemplate(const std::string& template_name, 
				const std::map<std::string, std::string>& variables = {}) const;

	// 验证配置
	bool ValidateConfig() const;

	// 获取错误信息
	const std::string& GetError() const { return error_message_; }

	// 保存配置到文件
	bool SaveToFile(const boost::filesystem::path& filename) const;

	// 生成配置JSON字符串
	std::string ToJsonString(bool pretty = true) const;

private:
	CodeGenConfig::ProjectConfig project_config_;
	std::map<std::string, std::string> variables_;
	std::string error_message_;

	void BuildVariableMap();
	std::string ProcessTemplate(const std::string& template_text) const;
	void SetError(const std::string& error);

public:
	// JSON辅助方法
	static std::vector<std::string> JsonArrayToStringVector(const json::value& array);
	static std::map<std::string, std::string> JsonObjectToStringMap(const json::value& object);
	static json::value StringVectorToJsonArray(const std::vector<std::string>& vec);
	static json::value StringMapToJsonObject(const std::map<std::string, std::string>& map);
};

} // namespace code_generator

#endif
