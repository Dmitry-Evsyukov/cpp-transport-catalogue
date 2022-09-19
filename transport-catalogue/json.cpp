#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node::Value LoadNode(istream& input);

        using Number = std::variant<int, double>;

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            if (input.peek() == '0') {
                read_char();
            } else {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    try {
                        return std::stoi(parsed_num);
                    } catch (...) {
                    }
                }
                return std::stod(parsed_num);
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                } else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        json::Node::Value LoadArray(istream& input) {
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return result;
        }

        json::Node::Value LoadDict(istream& input) {
            Dict result;

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input);
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }

            return result;
        }

        json::Node::Value LoadNode(istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            if (it == end) {
                return nullptr;
            }
            char c;
            input >> c;

            if (c == '[') {
                if (it == end) {
                    throw ParsingError("");
                }
                return LoadArray(input);
            } else if (c == '{') {
                if (it == end) {
                    throw ParsingError("");
                }
                return LoadDict(input);
            } else if (c == '\"') {
                return LoadString(input);
            } else if (c == 'n') {
                string nullptr_str = "n";
                while (nullptr_str != "null") {
                    if (it == end) {
                        throw ParsingError("");
                    }
                    input >> c;
                    nullptr_str.push_back(c);
                }
                return nullptr;
            } else if (c == 't') {
                string true_str = "t";
                while (true_str != "true") {
                    if (it == end) {
                        throw ParsingError("");
                    }
                    input >> c;
                    true_str.push_back(c);
                }
                return true;
            } else if (c == 'f') {
                string false_str = "f";
                while (false_str != "false") {
                    if (it == end) {
                        throw ParsingError("");
                    }
                    input >> c;
                    false_str.push_back(c);
                }
                return false;
            } else if (c == ' ') {
                return LoadNode(input);
            } else {
                input.putback(c);
                auto num = LoadNumber(input);
                if (holds_alternative<int>(num)) {
                    return get<int>(num);
                }
                return get<double>(num);
            }
        }

    }  // namespace



    const Array& Node::AsArray() const {
        if (holds_alternative<Array>(value_)) {
            return get<Array>(value_);
        }
        throw logic_error(""s);
    }

    const Dict& Node::AsMap() const {
        if (holds_alternative<Dict>(value_)) {
            return get<Dict>(value_);
        }
        throw logic_error(""s);
    }

    int Node::AsInt() const {
        if (holds_alternative<int>(value_)) {
            return get<int>(value_);
        }
        throw logic_error(""s);
    }

    const string& Node::AsString() const {
        if (holds_alternative<string>(value_)) {
            return get<string>(value_);
        }
        throw logic_error(""s);
    }

    double Node::AsDouble() const {
        if (holds_alternative<double>(value_)) {
            return get<double>(value_);
        }
        if (holds_alternative<int>(value_)) {
            return get<int>(value_);
        }
        throw logic_error(""s);
    }

    bool Node::IsInt() const { return holds_alternative<int>(value_);}
    bool Node::IsDouble() const {return holds_alternative<double>(value_) || holds_alternative<int>(value_);;}
    bool Node::IsPureDouble() const {return holds_alternative<double>(value_);}
    bool Node::IsBool() const {return holds_alternative<bool>(value_);}
    bool Node::IsString() const {return holds_alternative<string>(value_);}
    bool Node::IsNull() const {return holds_alternative<nullptr_t>(value_);}
    bool Node::IsArray() const {return holds_alternative<Array>(value_);}
    bool Node::IsMap() const {return holds_alternative<Dict>(value_);}

    bool Node::AsBool() const {
        if (holds_alternative<bool>(value_)) {
            return get<bool>(value_);
        }
        throw logic_error(""s);
    }

    Document::Document(Node root)
            : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{Node(LoadNode(input))};
    }

    struct VisitorValueOutput {
        std::ostream& out;
        void operator()(std::nullptr_t) {
            out << "null";
        }
        void operator()(bool val) {
            if (val) {
                out << "true"s;
            } else {
                out << "false"s;
            }
        }
        void operator()(int val) {
            out << val;
        }
        void operator()(double val) {
            out << val;
        }
        void operator()(const string& val) {
            string buffer;
            buffer += "\""s;
            for (const char c: val) {
                if (c == '\\' ) {
                    buffer += "\\\\"s;
                } else if (c == '\r') {
                    buffer += "\\r"s;
                } else if (c == '\"') {
                    buffer += "\\\""s;
                } else if (c == '\n') {
                    buffer += "\\n"s;
                } else {
                    buffer += c;
                }
            }
            buffer += "\""s;
            out << buffer;
        }
        void operator()(Array val) {
            out << '[';
            bool flag = false;
            for (const auto& node : val) {
                if (!flag) {
                    visit(VisitorValueOutput{out}, node.GetValue());
                    flag = true;
                } else {
                    out << ",";
                    visit(VisitorValueOutput{out}, node.GetValue());
                }


            }
            out << ']';
        }
        void operator()(Dict val) {
            out << "{ "s;
            bool flag = false;
            for (const auto& [key, value] : val) {
                if (!flag) {
                    out << '\"' << key << "\": "s;
                    visit(VisitorValueOutput{out}, value.GetValue());
                    flag = true;
                } else {
                    out << ", ";
                    out << '\"' << key << "\": "s;
                    visit(VisitorValueOutput{out}, value.GetValue());
                }
            }
            out << '}';
        }
    };
    void Print(const Document& doc, std::ostream& output) {
        visit(VisitorValueOutput{output}, doc.GetRoot().GetValue());
    }



}  // namespace json