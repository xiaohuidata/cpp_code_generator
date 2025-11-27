#include "code_generator/config_parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>

#include <iostream>

namespace code_generator {

// ClassConfig实现
CodeGenConfig::ClassConfig CodeGenConfig::ClassConfig::FromJson(const json::value& json) {
	ClassConfig config;

	if (json.is_object()) {
		const json::object& obj = json.as_object();

		// 名称
		if (obj.contains("name") && obj.at("name").is_string()) {
			config.name = obj.at("name").as_string().c_str();
		}

		// 基类
		if (obj.contains("base_classes") && obj.at("base_classes").is_array()) {
			config.base_classes = code_generator::ConfigParser::JsonArrayToStringVector(obj.at("base_classes"));
		}

		// 模板
		if (obj.contains("templates") && obj.at("templates").is_array()) {
			config.templates = code_generator::ConfigParser::JsonArrayToStringVector(obj.at("templates"));
		}

		// 元数据
		if (obj.contains("metadata") && obj.at("metadata").is_object()) {
			config.metadata = code_generator::ConfigParser::JsonObjectToStringMap(obj.at("metadata"));
		}
	}

	return config;
}

json::value CodeGenConfig::ClassConfig::ToJson() const {
	json::object obj;
	obj["name"] = name;
	obj["base_classes"] = ConfigParser::StringVectorToJsonArray(base_classes);
	obj["templates"] = ConfigParser::StringVectorToJsonArray(templates);
	obj["metadata"] = ConfigParser::StringMapToJsonObject(metadata);
	return obj;
}

// FunctionConfig实现
CodeGenConfig::FunctionConfig CodeGenConfig::FunctionConfig::FromJson(const json::value& json) {
	FunctionConfig config;

	if (json.is_object()) {
		const json::object& obj = json.as_object();

		// 基本信息
		if (obj.contains("name") && obj.at("name").is_string()) {
			config.name = obj.at("name").as_string().c_str();
		}
		if (obj.contains("return_type") && obj.at("return_type").is_string()) {
			config.return_type = obj.at("return_type").as_string().c_str();
		}
		if (obj.contains("body") && obj.at("body").is_string()) {
			config.body = obj.at("body").as_string().c_str();
		}
		if (obj.contains("access") && obj.at("access").is_string()) {
			config.access = obj.at("access").as_string().c_str();
		}

		// 布尔属性
		config.is_virtual = obj.contains("virtual") && obj.at("virtual").is_bool() && obj.at("virtual").as_bool();
		config.is_pure_virtual = obj.contains("pure_virtual") && obj.at("pure_virtual").is_bool() && obj.at("pure_virtual").as_bool();
		config.is_const = obj.contains("const") && obj.at("const").is_bool() && obj.at("const").as_bool();
		config.is_static = obj.contains("static") && obj.at("static").is_bool() && obj.at("static").as_bool();

		// 模板
		if (obj.contains("templates") && obj.at("templates").is_array()) {
			config.templates = ConfigParser::JsonArrayToStringVector(obj.at("templates"));
		}

		// 参数
		if (obj.contains("parameters") && obj.at("parameters").is_array()) {
			const json::array& params = obj.at("parameters").as_array();
			for (const auto& param : params) {
				if (param.is_object()) {
					const json::object& param_obj = param.as_object();
					std::string type, name;

					if (param_obj.contains("type") && param_obj.at("type").is_string()) {
						type = param_obj.at("type").as_string().c_str();
					}
					if (param_obj.contains("name") && param_obj.at("name").is_string()) {
						name = param_obj.at("name").as_string().c_str();
					}

					if (!type.empty() && !name.empty()) {
						config.parameters.emplace_back(type, name);
					}
				}
			}
		}
	}

	return config;
}

json::value CodeGenConfig::FunctionConfig::ToJson() const {
	json::object obj;
	obj["name"] = name;
	obj["return_type"] = return_type;
	obj["body"] = body;
	obj["access"] = access;
	obj["virtual"] = is_virtual;
	obj["pure_virtual"] = is_pure_virtual;
	obj["const"] = is_const;
	obj["static"] = is_static;
	obj["templates"] = ConfigParser::StringVectorToJsonArray(templates);

	// 参数数组
	json::array params_array;
	//for (const auto& [type, name] : parameters) {
	for (const auto& iter : parameters) {
		const std::string& type = iter.first;
		const std::string& name = iter.second;
		json::object param_obj;
		param_obj["type"] = type;
		param_obj["name"] = name;
		params_array.push_back(param_obj);
	}
	obj["parameters"] = params_array;

	return obj;
}

// MemberConfig实现
CodeGenConfig::MemberConfig CodeGenConfig::MemberConfig::FromJson(const json::value& json) {
	MemberConfig config;

	if (json.is_object()) {
		const json::object& obj = json.as_object();

		if (obj.contains("name") && obj.at("name").is_string()) {
			config.name = obj.at("name").as_string().c_str();
		}
		if (obj.contains("type") && obj.at("type").is_string()) {
			config.type = obj.at("type").as_string().c_str();
		}
		if (obj.contains("initializer") && obj.at("initializer").is_string()) {
			config.initializer = obj.at("initializer").as_string().c_str();
		}
		if (obj.contains("access") && obj.at("access").is_string()) {
			config.access = obj.at("access").as_string().c_str();
		}
		if (obj.contains("comment") && obj.at("comment").is_string()) {
			config.comment = obj.at("comment").as_string().c_str();
		}
	}

	return config;
}

json::value CodeGenConfig::MemberConfig::ToJson() const {
	json::object obj;
	obj["name"] = name;
	obj["type"] = type;
	obj["initializer"] = initializer;
	obj["access"] = access;
	obj["comment"] = comment;
	return obj;
}

// FileConfig实现
CodeGenConfig::FileConfig CodeGenConfig::FileConfig::FromJson(const json::value& json) {
	FileConfig config;

	if (json.is_object()) {
		const json::object& obj = json.as_object();

		// 基本属性
		if (obj.contains("filename") && obj.at("filename").is_string()) {
			config.filename = obj.at("filename").as_string().c_str();
		}
		if (obj.contains("type") && obj.at("type").is_string()) {
			config.type = obj.at("type").as_string().c_str();
		}

		// 数组属性
		if (obj.contains("includes") && obj.at("includes").is_array()) {
			config.includes = ConfigParser::JsonArrayToStringVector(obj.at("includes"));
		}
		if (obj.contains("namespaces") && obj.at("namespaces").is_array()) {
			config.namespaces = ConfigParser::JsonArrayToStringVector(obj.at("namespaces"));
		}
		if (obj.contains("copy_files") && obj.at("copy_files").is_array()) {
			config.copy_files = ConfigParser::JsonArrayToStringVector(obj.at("copy_files"));
		}
		if (obj.contains("insert_snippets") && obj.at("insert_snippets").is_array()) {
			config.insert_snippets = ConfigParser::JsonArrayToStringVector(obj.at("insert_snippets"));
		}

		// 类数组
		if (obj.contains("classes") && obj.at("classes").is_array()) {
			const json::array& classes = obj.at("classes").as_array();
			for (const auto& class_json : classes) {
				config.classes.push_back(ClassConfig::FromJson(class_json));
			}
		}

		// 函数数组
		if (obj.contains("functions") && obj.at("functions").is_array()) {
			const json::array& functions = obj.at("functions").as_array();
			for (const auto& func_json : functions) {
				config.functions.push_back(FunctionConfig::FromJson(func_json));
			}
		}

		// 全局变量数组
		if (obj.contains("globals") && obj.at("globals").is_array()) {
			const json::array& globals = obj.at("globals").as_array();
			for (const auto& global_json : globals) {
				config.globals.push_back(MemberConfig::FromJson(global_json));
			}
		}

		// 模板映射
		if (obj.contains("templates") && obj.at("templates").is_object()) {
			config.templates = ConfigParser::JsonObjectToStringMap(obj.at("templates"));
		}
	}

	return config;
}

json::value CodeGenConfig::FileConfig::ToJson() const {
	json::object obj;
	obj["filename"] = filename;
	obj["type"] = type;
	obj["includes"] = ConfigParser::StringVectorToJsonArray(includes);
	obj["namespaces"] = ConfigParser::StringVectorToJsonArray(namespaces);
	obj["copy_files"] = ConfigParser::StringVectorToJsonArray(copy_files);
	obj["insert_snippets"] = ConfigParser::StringVectorToJsonArray(insert_snippets);

	// 类数组
	json::array classes_array;
	for (const auto& class_config : classes) {
		classes_array.push_back(class_config.ToJson());
	}
	obj["classes"] = classes_array;

	// 函数数组
	json::array functions_array;
	for (const auto& func_config : functions) {
		functions_array.push_back(func_config.ToJson());
	}
	obj["functions"] = functions_array;

	// 全局变量数组
	json::array globals_array;
	for (const auto& global_config : globals) {
		globals_array.push_back(global_config.ToJson());
	}
	obj["globals"] = globals_array;

	obj["templates"] = ConfigParser::StringMapToJsonObject(templates);

	return obj;
}

// ProjectConfig实现
CodeGenConfig::ProjectConfig CodeGenConfig::ProjectConfig::FromJson(const json::value& json) {
	ProjectConfig config;

	if (json.is_object()) {
		const json::object& obj = json.as_object();

		// 基本属性
		if (obj.contains("name") && obj.at("name").is_string()) {
			config.name = obj.at("name").as_string().c_str();
		}
		if (obj.contains("version") && obj.at("version").is_string()) {
			config.version = obj.at("version").as_string().c_str();
		}
		if (obj.contains("output_dir") && obj.at("output_dir").is_string()) {
			config.output_dir = obj.at("output_dir").as_string().c_str();
		}

		// 公共包含
		if (obj.contains("common_includes") && obj.at("common_includes").is_array()) {
			config.common_includes = ConfigParser::JsonArrayToStringVector(obj.at("common_includes"));
		}

		// 文件数组
		if (obj.contains("files") && obj.at("files").is_array()) {
			const json::array& files = obj.at("files").as_array();
			for (const auto& file_json : files) {
				config.files.push_back(FileConfig::FromJson(file_json));
			}
		}

		// 变量映射
		if (obj.contains("variables") && obj.at("variables").is_object()) {
			config.variables = ConfigParser::JsonObjectToStringMap(obj.at("variables"));
		}

		// 代码模板
		if (obj.contains("code_templates") && obj.at("code_templates").is_object()) {
			config.code_templates = ConfigParser::JsonObjectToStringMap(obj.at("code_templates"));
		}
	}

	return config;
}

void CodeGenConfig::LoadVariables(const json::value& json, std::map<std::string, std::string>& variables) {
	if (json.is_object()) {
		const json::object& obj = json.as_object();

		// 变量映射
		if (obj.contains("variables") && obj.at("variables").is_object()) {
			variables = ConfigParser::JsonObjectToStringMap(obj.at("variables"));
		}
	}
}

json::value CodeGenConfig::ProjectConfig::ToJson() const {
	json::object obj;
	obj["name"] = name;
	obj["version"] = version;
	obj["output_dir"] = output_dir;
	obj["common_includes"] = ConfigParser::StringVectorToJsonArray(common_includes);

	// 文件数组
	json::array files_array;
	for (const auto& file_config : files) {
		files_array.push_back(file_config.ToJson());
	}
	obj["files"] = files_array;

	obj["variables"] = ConfigParser::StringMapToJsonObject(variables);
	obj["code_templates"] = ConfigParser::StringMapToJsonObject(code_templates);

	return obj;
}

// ConfigParser实现
ConfigParser::ConfigParser() : error_message_("") {}

bool ConfigParser::LoadFromFile(const boost::filesystem::path& filename) {
	try {
		if (!boost::filesystem::exists(filename)) {
			SetError("Config file does not exist: " + filename.string());
			return false;
		}

		std::ifstream file(filename.string());
		if (!file.is_open()) {
			SetError("Cannot open config file: " + filename.string());
			return false;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();

		json::value json;
		try {
			json = json::parse(buffer.str());
		} catch (const std::exception& e) {
			SetError("JSON parsing error: " + std::string(e.what()));
			return false;
		}

		std::map<std::string, std::string> variables;
		CodeGenConfig::LoadVariables(json, variables);
		std::string strbuffer = buffer.str();
		std::cout << "variables.size()" << variables.size() << std::endl;
		if (variables.size()) {
			ReplaceBufferByVariables(strbuffer, variables);
			try {
				json = json::parse(strbuffer);
			} catch (const std::exception& e) {
				SetError("JSON2 parsing error: " + std::string(e.what()));
				return false;
			}
		}
		return LoadFromJson(json);

	} catch (const std::exception& e) {
		SetError("File loading error: " + std::string(e.what()));
		return false;
	}
}

bool ConfigParser::LoadFromString(const std::string& json_str) {
	try {
		json::value json = json::parse(json_str);
		return LoadFromJson(json);
	} catch (const std::exception& e) {
		SetError("JSON parsing error: " + std::string(e.what()));
		return false;
	}
}

bool ConfigParser::LoadFromJson(const json::value& json) {
	try {
		project_config_ = CodeGenConfig::ProjectConfig::FromJson(json);
		BuildVariableMap();

		if (!ValidateConfig()) {
			return false;
		}

		error_message_.clear();
		return true;

	} catch (const std::exception& e) {
		SetError("Config loading error: " + std::string(e.what()));
		return false;
	}
}

std::string ConfigParser::ReplaceVariables(const std::string& text) const {
	std::string result = text;

	//for (const auto& [key, value] : variables_) {
	for (const auto& iter : variables_) {
		const std::string& key = iter.first;
		const std::string& value = iter.second;
		std::string placeholder = "${" + key + "}";
		size_t pos = 0;
		while ((pos = result.find(placeholder, pos)) != std::string::npos) {
			result.replace(pos, placeholder.length(), value);
			pos += value.length();
		}
	}

	// 替换系统变量
	auto replace_system_var = [&](const std::string& placeholder, const std::string& value) {
		size_t pos = 0;
		while ((pos = result.find(placeholder, pos)) != std::string::npos) {
			result.replace(pos, placeholder.length(), value);
			pos += value.length();
		}
	};

	replace_system_var("${PROJECT_NAME}", project_config_.name);
	replace_system_var("${PROJECT_VERSION}", project_config_.version);
	replace_system_var("${OUTPUT_DIR}", project_config_.output_dir);

	return result;
}

void ConfigParser::ReplaceBufferByVariables(std::string& strjson, std::map<std::string, std::string>& variables) {
	for (const auto& iter : variables) {
		const std::string& key = iter.first;
		const std::string& value = iter.second;
		std::string placeholder = "${" + key + "}";
		size_t pos = 0;
		while ((pos = strjson.find(placeholder, pos)) != std::string::npos) {
			strjson.replace(pos, placeholder.length(), value);
			pos += value.length();
		}
	}
}

std::string ConfigParser::GetTemplate(const std::string& name) const {
	auto it = project_config_.code_templates.find(name);
	if (it != project_config_.code_templates.end()) {
		return ProcessTemplate(it->second);
	}
	return "";
}

std::string ConfigParser::ApplyTemplate(const std::string& template_name, 
			const std::map<std::string, std::string>& variables) const {
	std::string template_content = GetTemplate(template_name);
	if (template_content.empty()) {
		return "";
	}

	std::string result = template_content;

	// 替换传入的变量
	//for (const auto& [key, value] : variables) {
	for (const auto& iter : variables) {
		const std::string& key = iter.first;
		const std::string& value = iter.second;
		std::string placeholder = "${" + key + "}";
		size_t pos = 0;
		while ((pos = result.find(placeholder, pos)) != std::string::npos) {
			result.replace(pos, placeholder.length(), value);
			pos += value.length();
		}
	}

	return result;
}

bool ConfigParser::ValidateConfig() const {
	if (project_config_.name.empty()) {
		SetError("Project name is required");
		return false;
	}

	if (project_config_.files.empty()) {
		SetError("At least one file must be specified");
		return false;
	}

	for (const auto& file_config : project_config_.files) {
		if (file_config.filename.empty()) {
			SetError("Filename cannot be empty");
			return false;
		}

		// 检查文件名有效性
		if (file_config.filename.find("..") != std::string::npos) {
			SetError("Invalid filename: " + file_config.filename);
			return false;
		}
	}

	return true;
}

bool ConfigParser::SaveToFile(const boost::filesystem::path& filename) const {
	try {
		std::ofstream file(filename.string());
		if (!file.is_open()) {
			SetError("Cannot open file for writing: " + filename.string());
			return false;
		}

		std::string json_str = ToJsonString(true);
		file << json_str;
		file.close();

		return true;
	} catch (const std::exception& e) {
		SetError("File saving error: " + std::string(e.what()));
		return false;
	}
}

std::string ConfigParser::ToJsonString(bool pretty) const {
	json::value json = project_config_.ToJson();

	if (pretty) {
		//return json::serialize(json, json::serialize_opts{});
		return json::serialize(json);
	} else {
		return json::serialize(json);
	}
}

void ConfigParser::BuildVariableMap() {
	variables_ = project_config_.variables;

	// 添加默认变量
	variables_["PROJECT_NAME"] = project_config_.name;
	variables_["PROJECT_VERSION"] = project_config_.version;
	variables_["OUTPUT_DIR"] = project_config_.output_dir;

	// 添加时间戳变量
	std::time_t now = std::time(nullptr);
	char time_str[100];
	std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
	variables_["TIMESTAMP"] = time_str;
}

std::string ConfigParser::ProcessTemplate(const std::string& template_text) const {
	return ReplaceVariables(template_text);
}

void ConfigParser::SetError(const std::string& error) {
	error_message_ = error;
}

// JSON辅助方法实现
std::vector<std::string> ConfigParser::JsonArrayToStringVector(const json::value& array) {
	std::vector<std::string> result;

	if (array.is_array()) {
		const json::array& arr = array.as_array();
		for (const auto& item : arr) {
			if (item.is_string()) {
				result.push_back(item.as_string().c_str());
			}
		}
	}

	return result;
}

std::map<std::string, std::string> ConfigParser::JsonObjectToStringMap(const json::value& object) {
	std::map<std::string, std::string> result;

	if (object.is_object()) {
		const json::object& obj = object.as_object();
		/*
		for (const auto& [key, value] : obj) {
			if (value.is_string()) {
				result[key.data()] = value.as_string().c_str();
			}
		}
		*/
		for (const auto& iter : obj) {
			if (iter.value().is_string()) {
				result[iter.key()] = iter.value().as_string().c_str();
			}
		}
	}

	return result;
}

json::value ConfigParser::StringVectorToJsonArray(const std::vector<std::string>& vec) {
	json::array result;
	for (const auto& str : vec) {
		//result.push_back(str);
		result.emplace_back(json::value_from(str));
	}
	return result;
}

json::value ConfigParser::StringMapToJsonObject(const std::map<std::string, std::string>& map) {
	json::object result;
	//for (const auto& [key, value] : map) {
	for (const auto iter : map) {
		result[iter.first] = iter.second;
	}
	return result;
}

} // namespace code_generator
