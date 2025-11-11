#include "code_generator/formatter.h"
#include <boost/algorithm/string/join.hpp>

namespace code_generator {

Formatter::Formatter(ZeroCopyOutputStreamPtr output, IndentStyle style, bool use_braces)
	: output_(std::move(output)), indent_style_(style), use_braces_(use_braces),
	indent_level_(0), at_start_of_line_(true) {
}

Formatter::~Formatter() {
	Flush();
}

Formatter& Formatter::Print(const std::string& text) {
	if (text.empty()) return *this;

	if (at_start_of_line_) {
		WriteIndent();
		at_start_of_line_ = false;
	}

	WriteString(text);
	return *this;
}

Formatter& Formatter::Print(const char* text) {
	return Print(std::string(text));
}

Formatter& Formatter::Print(int value) {
	return Print(std::to_string(value));
}

Formatter& Formatter::Print(const std::vector<std::string>& lines) {
	for (const auto& line : lines) {
		AddLine(line);
	}
	return *this;
}

Formatter& Formatter::Indent() {
	++indent_level_;
	return *this;
}

Formatter& Formatter::Outdent() {
	if (indent_level_ > 0) {
		--indent_level_;
	}
	return *this;
}

Formatter& Formatter::SetIndentLevel(int level) {
	indent_level_ = std::max(0, level);
	return *this;
}

std::unique_ptr<Formatter::Scope> Formatter::OpenBlock(const std::string& prefix) {
	if (!prefix.empty()) {
		AddLine(prefix);
	}

	if (use_braces_) {
		AddLine("{");
		Indent();
	}

	return std::unique_ptr<Scope>(new Scope(this));
}

void Formatter::OpenBlockInternal(const std::string& prefix) {
	if (!prefix.empty()) {
		AddLine(prefix);
	}

	if (use_braces_) {
		AddLine("{");
		Indent();
	}
}

void Formatter::CloseBlock(const std::string& suffix) {
	if (use_braces_) {
		Outdent();
		AddLine("}" + suffix);
	} else {
		AddLine(suffix);
	}
}

Formatter& Formatter::EndLine() {
	output_->WriteChar('\n');
	at_start_of_line_ = true;
	return *this;
}

Formatter& Formatter::AddLine(const std::string& line) {
	if (!line.empty()) {
		Print(line);
	}
	return EndLine();
}

Formatter& Formatter::AddComment(const std::string& comment) {
	if (comment.find('\n') == std::string::npos) {
		return Print("// ").Print(comment).EndLine();
	} else {
		AddLine("/*");
		Indent();

		std::vector<std::string> lines;
		boost::split(lines, comment, boost::is_any_of("\n"));
		for (const auto& line : lines) {
			AddLine(line);
		}

		Outdent();
		return AddLine("*/");
	}
}

// 控制结构实现
Formatter& Formatter::If(const std::string& condition) {
	AddLine("if (" + condition + ")");
	block_stack_.push({BlockType::IF, condition});
	OpenBlockInternal();
	return *this;
}

Formatter& Formatter::Else() {
	if (!block_stack_.empty() && block_stack_.top().type == BlockType::IF) {
		CloseBlock(" else");
		block_stack_.top().type = BlockType::ELSE;
		OpenBlockInternal();
		return *this;
	}
	return *this;
}

Formatter& Formatter::ElseIf(const std::string& condition) {
	if (!block_stack_.empty() && 
				(block_stack_.top().type == BlockType::IF || block_stack_.top().type == BlockType::ELSE)) {
		CloseBlock(" else if (" + condition + ")");
		block_stack_.top() = {BlockType::IF, condition};
		OpenBlockInternal();
		return *this;
	}
	return *this;
}

Formatter& Formatter::EndIf() {
	if (!block_stack_.empty() && 
				(block_stack_.top().type == BlockType::IF || block_stack_.top().type == BlockType::ELSE)) {
		CloseBlock();
		block_stack_.pop();
	}
	return *this;
}

Formatter& Formatter::For(const std::string& loop_header) {
	AddLine("for (" + loop_header + ")");
	block_stack_.push({BlockType::FOR, loop_header});
	OpenBlockInternal();
	return *this;
}

Formatter& Formatter::While(const std::string& condition) {
	AddLine("while (" + condition + ")");
	block_stack_.push({BlockType::WHILE, condition});
	OpenBlockInternal();
	return *this;
}

Formatter& Formatter::EndLoop() {
	if (!block_stack_.empty() && 
				(block_stack_.top().type == BlockType::FOR || block_stack_.top().type == BlockType::WHILE)) {
		CloseBlock();
		block_stack_.pop();
	}
	return *this;
}

Formatter& Formatter::Class(const std::string& name, const std::string& inheritance) {
	std::string line = "class " + name;
	if (!inheritance.empty()) {
		line += " : " + inheritance;
	}
	AddLine(line);
	block_stack_.push({BlockType::CLASS, name});
	OpenBlockInternal();
	return *this;
}

Formatter& Formatter::Struct(const std::string& name, const std::string& inheritance) {
	std::string line = "struct " + name;
	if (!inheritance.empty()) {
		line += " : " + inheritance;
	}
	AddLine(line);
	block_stack_.push({BlockType::STRUCT, name});
	OpenBlockInternal();
	return *this;
}

Formatter& Formatter::EndClass() {
	if (!block_stack_.empty() && 
				(block_stack_.top().type == BlockType::CLASS || block_stack_.top().type == BlockType::STRUCT)) {
		CloseBlock(";");
		block_stack_.pop();
	}
	return *this;
}

Formatter& Formatter::Namespace(const std::string& name) {
	AddLine("namespace " + name + " {");
	block_stack_.push({BlockType::NAMESPACE, name});
	Indent();
	return *this;
}

Formatter& Formatter::EndNamespace() {
	if (!block_stack_.empty() && block_stack_.top().type == BlockType::NAMESPACE) {
		Outdent();
		AddLine("} // namespace " + block_stack_.top().prefix);
		block_stack_.pop();
	}
	return *this;
}

Formatter& Formatter::Enum(const std::string& name, const std::vector<std::string>& values) {
	std::string enum_def = "enum";
	if (name.find("class") == std::string::npos) {
		enum_def += " class";
	}
	enum_def += " " + name;

	OpenBlockInternal(enum_def);
	for (size_t i = 0; i < values.size(); ++i) {
		if (i == values.size() - 1) {
			AddLine(values[i]);
		} else {
			AddLine(values[i] + ",");
		}
	}
	CloseBlock(";");
	return *this;
}

Formatter& Formatter::Public() {
	return Outdent().AddLine("public:").Indent();
}

Formatter& Formatter::Private() {
	return Outdent().AddLine("private:").Indent();
}

Formatter& Formatter::Protected() {
	return Outdent().AddLine("protected:").Indent();
}

Formatter& Formatter::Include(const std::string& header) {
	return AddLine("#include " + header);
}

Formatter& Formatter::Define(const std::string& macro) {
	return AddLine("#define " + macro);
}

Formatter& Formatter::IfDef(const std::string& macro) {
	return AddLine("#ifdef " + macro);
}

Formatter& Formatter::IfNDef(const std::string& macro) {
	return AddLine("#ifndef " + macro);
}

Formatter& Formatter::EndIfDef() {
	return AddLine("#endif");
}

void Formatter::WriteIndent() {
	if (indent_level_ <= 0) return;

	if (indent_style_ == IndentStyle::TABS) {
		output_->WriteString(std::string(indent_level_, '\t'));
	} else {
		int spaces = indent_level_ * static_cast<int>(indent_style_);
		output_->WriteString(std::string(spaces, ' '));
	}
}

void Formatter::WriteString(const std::string& str) {
	output_->WriteString(str);
}

std::string Formatter::CurrentIndent() const {
	if (indent_style_ == IndentStyle::TABS) {
		return std::string(indent_level_, '\t');
	} else {
		return std::string(indent_level_ * static_cast<int>(indent_style_), ' ');
	}
}

void Formatter::Flush() {
	output_->Flush();
}

// Scope实现
Formatter::Scope::Scope(Formatter* formatter, const std::string& prefix)
		: formatter_(formatter) {
	if (!prefix.empty()) {
		formatter_->AddLine(prefix);
	}
	formatter_->Indent();
}

Formatter::Scope::~Scope() {
	if (formatter_) {
		formatter_->Outdent();
		formatter_->CloseBlock();
	}
}

} // namespace code_generator
