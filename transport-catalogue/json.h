#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

        Node() : value_(nullptr) {}
        Node(Value val) : value_(val) {}
        Node(Array arr) : value_(arr) {}
        Node(double arr) : value_(arr) {}
        Node(int arr) : value_(arr) {}
        Node(std::string arr) : value_(arr) {}
        Node(std::nullptr_t) : value_(nullptr) {}
        Node(bool arr) : value_(arr) {}
        Node(Dict arr) : value_(arr) {}

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;
        double AsDouble() const;
        bool AsBool() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int Index() const {
            return value_.index();
        }

        Value GetValue() const {
            return value_;
        }

    private:
        Value value_;
    };
    inline bool operator==(const Node& lhs, const Node& rhs) {
        if (lhs.Index() != rhs.Index()) {
            return false;
        }
        return lhs.GetValue() == rhs.GetValue();
    }

    inline bool operator != (const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }


    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    inline bool operator == (const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator != (const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() != rhs.GetRoot();
    }

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json