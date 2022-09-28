#include "json_builder.h"

using namespace std;

namespace json {
    Builder &Builder::Value(Node node) {
        if (flag_value) throw logic_error("wrong argument");
        if (nodes_stack_.empty()) {
            flag_value = true;
            root_ = move(node);
            return *this;
        }

        Node* last_node = nodes_stack_[nodes_stack_.size() - 1];
        if (last_node->IsString()) {
            auto last_dict = nodes_stack_[nodes_stack_.size() - 2];
            get<Dict>(last_dict->GetValue())[last_node->AsString()] = move(node);
            nodes_stack_.pop_back();
        } else if (last_node->IsArray()) {
            get<Array>(last_node->GetValue()).emplace_back(move(node));
        } else {
            throw logic_error("wrong argument");
        }
        return *this;
    }

    DictItemContext &Builder::ValueDict(Node node) {
        Value(node);
        return dict_item_context;
    }

    ArrayItemContext &Builder::ValueArray(Node node) {
        Value(node);
        return array_item_context;
    }

    KeyItemContext &Builder::Key(std::string key) {
        if (flag_value) throw logic_error("wrong argument");

        if (nodes_stack_.empty()) {
            throw logic_error("wrong argument");
        }
        Node* last_node = nodes_stack_[nodes_stack_.size() - 1];
        if (last_node->IsDict()) {
            get<Dict>(last_node->GetValue())[key];
            key_ = move(Node(move(key)));
            nodes_stack_.push_back(&key_);
        } else {
            throw logic_error("wrong argument");
        }
        return key_item_context;
    }

    Builder &Builder::EndArray() {
        if (flag_value) throw logic_error("wrong argument");

        if (nodes_stack_[nodes_stack_.size() - 1]->IsArray()) {
            nodes_stack_.pop_back();
            return *this;
        }
        throw logic_error("wrong argument");
    }

    Builder &Builder::EndDict() {
        if (flag_value) throw logic_error("wrong argument");

        if (nodes_stack_[nodes_stack_.size() - 1]->IsDict()) {
            nodes_stack_.pop_back();
            return *this;
        }
        throw logic_error("wrong argument");
    }

    ArrayItemContext &Builder::StartArray() {
        if (flag_value) throw logic_error("wrong argument");

        if (nodes_stack_.empty()) {
            root_ = Node(Array ());
            nodes_stack_.push_back(&root_);
            return array_item_context;
        }
        Node* last_node = nodes_stack_[nodes_stack_.size() - 1];
        if (last_node->IsString()) {
            auto last_dict = nodes_stack_[nodes_stack_.size() - 2];
            get<Dict>(last_dict->GetValue())[last_node->AsString()] = Array();
            auto s = last_node->AsString();
            nodes_stack_.pop_back();
            nodes_stack_.push_back(&get<Dict>(last_dict->GetValue())[s]);

        } else if (last_node->IsArray()) {
            get<Array>(last_node->GetValue()).emplace_back(Node(Array ()));
            nodes_stack_.push_back(&get<Array>(last_node->GetValue())[last_node->AsArray().size() - 1]);
        } else {
            throw logic_error("wrong argument");
        }
        return array_item_context;
    }

    DictItemContext &Builder::StartDict() {
        if (flag_value) throw logic_error("wrong argument");

        if (nodes_stack_.empty()) {
            root_ = Node(Dict());
            nodes_stack_.push_back(&root_);
            return dict_item_context;
        }

        Node* last_node = nodes_stack_[nodes_stack_.size() - 1];

        if (last_node->IsArray()) {
            get<Array>(last_node->GetValue()).emplace_back(Node(Dict()));
            nodes_stack_.push_back(&get<Array>(last_node->GetValue())[last_node->AsArray().size() - 1]);
        } else if (last_node->IsString()) {
            auto last_dict = nodes_stack_[nodes_stack_.size() - 2];
            get<Dict>(last_dict->GetValue())[last_node->AsString()] = Dict();
            auto s = last_node->AsString();
            nodes_stack_.pop_back();
            nodes_stack_.push_back(&get<Dict>(last_dict->GetValue())[s]);
        } else {
            throw logic_error("wrong argument");
        }

        return dict_item_context;
    }

    Node Builder::Build() const {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            return root_;
        } else {
            throw logic_error("this object can't be created");
        }
    }

    KeyItemContext &DictItemContext::Key(std::string key)  {
        return builder_.Key(key);
    }
    Builder &DictItemContext::EndDict() {
        return builder_.EndDict();
    }

    DictItemContext &KeyItemContext::StartDict() {
        return builder_.StartDict();
    }


    ArrayItemContext &KeyItemContext::StartArray() {
        return builder_.StartArray();
    }

    DictItemContext &KeyItemContext::Value(Node node) {
        return builder_.ValueDict(node);
    }

    ArrayItemContext &ArrayItemContext::Value(Node node) {
        return builder_.ValueArray(node);
    }

    DictItemContext &ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext &ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder &ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }
}