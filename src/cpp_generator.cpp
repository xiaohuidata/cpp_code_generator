#include "code_generator/cpp_generator.h"
#include <algorithm>

namespace code_generator {

std::string CppType::ToString() const {
    std::string result;
    if (is_const) {
        result += "const ";
    }
    result += name;
    if (is_pointer) {
        result += "*";
    }
    if (is_reference) {
        result += "&";
    }
    return result;
}

std::string CppParameter::ToString() const {
    std::string result = type.ToString() + " " + name;
    if (!default_value.empty()) {
        result += " = " + default_value;
    }
    return result;
}

std::string CppFunction::GetSignature() const {
    std::string result;
    
    if (is_virtual) {
        result += "virtual ";
    }
    if (is_static) {
        result += "static ";
    }
    
    result += return_type + " " + name + "(";
    
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) {
            result += ", ";
        }
        result += parameters[i].ToString();
    }
    
    result += ")";
    
    if (is_const) {
        result += " const";
    }
    
    if (is_pure_virtual) {
        result += " = 0";
    }
    
    return result;
}

std::string CppMember::ToString() const {
    std::string result = type.ToString() + " " + name;
    if (!initializer.empty()) {
        result += " = " + initializer;
    }
    return result + ";";
}

void CppClass::AddFunction(const CppFunction& func) {
    functions.push_back(func);
}

void CppClass::AddMember(const CppMember& member) {
    members.push_back(member);
}

CppGenerator::CppGenerator(code_generator::ZeroCopyOutputStreamPtr output, const CppGeneratorOptions& options)
    : formatter_(output, options.indent_style, options.use_braces),
      options_(options) {
}

void CppGenerator::BeginFile(const std::string& filename, const std::vector<std::string>& includes) {
    current_filename_ = filename;

    if (options_.generate_comments) {
        GenerateFileHeader(filename);
    }
    
    if (options_.use_pragma_once) {
        formatter_.AddLine("#pragma once");
    } else if (options_.use_include_guards) {
        GenerateIncludeGuards(true);
    }
    
    formatter_.EndLine();
    GenerateIncludes(includes);
    formatter_.EndLine();
}

void CppGenerator::EndFile() {
    if (options_.use_include_guards && !options_.use_pragma_once) {
        GenerateIncludeGuards(false);
    }
}

void CppGenerator::BeginNamespace(const std::string& name) {
    formatter_.Namespace(name);
}

void CppGenerator::EndNamespace() {
    formatter_.EndNamespace();
}

void CppGenerator::GenerateClass(const CppClass& cls) {
    GenerateClassDeclaration(cls);
    formatter_.EndLine();
    GenerateClassImplementation(cls);
}

void CppGenerator::GenerateClassDeclaration(const CppClass& cls) {
    // 前向声明
    for (const auto& decl : cls.forward_declarations) {
        formatter_.AddLine(decl);
    }
    if (!cls.forward_declarations.empty()) {
        formatter_.EndLine();
    }
    
    // 类定义
    std::string inheritance;
    if (!cls.base_classes.empty()) {
        inheritance = "public " + cls.base_classes[0];
        for (size_t i = 1; i < cls.base_classes.size(); ++i) {
            inheritance += ", public " + cls.base_classes[i];
        }
    }

    // 使用手动作用域管理而不是 OpenBlock
    formatter_.Class(cls.name, inheritance);

    // 成员变量和函数按访问权限分组
    std::map<std::string, std::vector<CppMember> > members_by_access;
    std::map<std::string, std::vector<CppFunction> > functions_by_access;

    for (const auto& member : cls.members) {
        members_by_access[member.access_specifier].push_back(member);
    }

    for (const auto& func : cls.functions) {
        functions_by_access[func.access_specifier].push_back(func);
    }

    // 生成public部分
    if (functions_by_access.count("public") || members_by_access.count("public")) {
        formatter_.Public();

        for (const auto& func : functions_by_access["public"]) {
            GenerateFunctionDeclaration(func);
        }

        for (const auto& member : members_by_access["public"]) {
            if (options_.generate_comments) {
                GenerateMemberComment(member);
            }
            formatter_.AddLine(member.ToString());
        }
    }

    // 生成protected部分
    if (functions_by_access.count("protected") || members_by_access.count("protected")) {
        formatter_.Protected();

        for (const auto& func : functions_by_access["protected"]) {
            GenerateFunctionDeclaration(func);
        }

        for (const auto& member : members_by_access["protected"]) {
            if (options_.generate_comments) {
                GenerateMemberComment(member);
            }
            formatter_.AddLine(member.ToString());
        }
    }

    // 生成private部分
    if (functions_by_access.count("private") || members_by_access.count("private")) {
        formatter_.Private();

        for (const auto& func : functions_by_access["private"]) {
            GenerateFunctionDeclaration(func);
        }

        for (const auto& member : members_by_access["private"]) {
            if (options_.generate_comments) {
                GenerateMemberComment(member);
            }
            formatter_.AddLine(member.ToString());
        }
    }

    // 作用域结束会自动调用CloseBlock
    formatter_.EndClass();
}

void CppGenerator::GenerateClassImplementation(const CppClass& cls, const std::string& namespace_prefix) {
    for (const auto& func : cls.functions) {
        if (!func.body.empty() && !func.is_pure_virtual) {
            GenerateFunctionImplementation(func, cls.name);
        }
    }
}

void CppGenerator::GenerateFunction(const CppFunction& func, bool in_class) {
    if (in_class) {
        GenerateFunctionDeclaration(func);
    } else {
        GenerateFunctionImplementation(func);
    }
}

void CppGenerator::GenerateFunctionDeclaration(const CppFunction& func) {
    if (options_.generate_comments) {
        GenerateFunctionComment(func);
    }
    formatter_.AddLine(func.GetSignature() + ";");
}

void CppGenerator::GenerateFunctionImplementation(const CppFunction& func, const std::string& class_name) {
    std::string signature = func.GetSignature();
    if (!class_name.empty()) {
        // 插入类名作用域
        size_t pos = signature.find(func.name);
        if (pos != std::string::npos) {
            signature.insert(pos, class_name + "::");
        }
    }
    
    // 使用手动作用域管理
    formatter_.AddLine(signature);
    formatter_.OpenBlockInternal();

    if (!func.body.empty()) {
        std::vector<std::string> lines;
        size_t start = 0;
        size_t end = func.body.find('\n');
        
        while (end != std::string::npos) {
            lines.push_back(func.body.substr(start, end - start));
            start = end + 1;
            end = func.body.find('\n', start);
        }
        if (start < func.body.length()) {
            lines.push_back(func.body.substr(start));
        }
        
        formatter_.Print(lines);
    } else {
        formatter_.AddComment("TODO: Implement function body");
    }
    formatter_.CloseBlock();
}

void CppGenerator::GenerateEnum(const std::string& name, const std::vector<std::string>& values, const std::string& type) {
    /*
    std::string enum_def = "enum";
    if (!type.empty()) {
        enum_def += " class";
    }
    enum_def += " " + name;
    
    if (!type.empty()) {
        enum_def += " : " + type;
    }
    
    auto scope = formatter_.OpenBlock(enum_def);
    
    for (size_t i = 0; i < values.size(); ++i) {
        if (i == values.size() - 1) {
            formatter_.AddLine(values[i]);
        } else {
            formatter_.AddLine(values[i] + ",");
        }
    }
    */
    formatter_.Enum(name, values);
}

void CppGenerator::GenerateGetter(const CppMember& member) {
    CppFunction getter;
    getter.return_type = "const " + member.type.ToString() + "&";
    getter.name = "Get" + member.name;
    getter.is_const = true;
    getter.body = "return " + member.name + ";";
    
    GenerateFunction(getter, true);
}

void CppGenerator::GenerateSetter(const CppMember& member) {
    CppFunction setter;
    setter.return_type = "void";
    setter.name = "Set" + member.name;
    setter.parameters.push_back({member.type, "value", ""});
    setter.body = member.name + " = value;";
    
    GenerateFunction(setter, true);
}

void CppGenerator::GenerateFileHeader(const std::string& filename) {
    if (!options_.file_header_comment.empty()) {
        formatter_.AddComment(options_.file_header_comment);
    } else {
        formatter_.AddComment("Generated by CppGenerator");
        formatter_.AddComment("File: " + filename);
    }
    formatter_.EndLine();
}

void CppGenerator::GenerateFunctionComment(const CppFunction& func) {
    std::string comment = func.name + " - ";
    if (!func.parameters.empty()) {
        comment += "Parameters: ";
        for (size_t i = 0; i < func.parameters.size(); ++i) {
            if (i > 0) comment += ", ";
            comment += func.parameters[i].name;
        }
    }
    formatter_.AddComment(comment);
}

void CppGenerator::GenerateMemberComment(const CppMember& member) {
    formatter_.AddComment(member.name + " - member variable");
}

void CppGenerator::GenerateIncludeGuards(bool begin) {
    std::string guard = BuildIncludeGuard(current_filename_);
    
    if (begin) {
        formatter_.AddLine("#ifndef " + guard);
        formatter_.AddLine("#define " + guard);
    } else {
        formatter_.AddLine("#endif // " + guard);
    }
}

void CppGenerator::GenerateIncludes(const std::vector<std::string>& includes) {
    for (const auto& include : includes) {
        if (include.find('<') != std::string::npos || include.find('>') != std::string::npos) {
            formatter_.Include(include);
        } else {
            formatter_.Include("\"" + include + "\"");
        }
    }
}

std::string CppGenerator::BuildIncludeGuard(const std::string& filename) {
    std::string guard = options_.include_guard_prefix;
    
    for (char c : filename) {
        if (std::isalnum(c)) {
            guard += std::toupper(c);
        } else if (c == '.' || c == '/') {
            guard += '_';
        }
    }
    
    guard += "_";
    return guard;
}

} // namespace code_generator
