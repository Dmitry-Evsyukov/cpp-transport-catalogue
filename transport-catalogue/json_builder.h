#pragma once
#include "json.h"

namespace json {
    class Builder;
    class KeyItemContext;
    class DictItemContext;

    class ArrayItemContext{
    public:
        ArrayItemContext(Builder& builder) : builder_(builder) {}
        ArrayItemContext& Value(Node node);
        Builder& EndArray();
        ArrayItemContext& StartArray();
        DictItemContext& StartDict();
    private:
        Builder& builder_;
    };

    class DictItemContext {
    public:
        DictItemContext( Builder& builder) : builder_(builder) {}
        KeyItemContext& Key(std::string key);
        Builder& EndDict();
    private:
        Builder& builder_;
        Builder* parent_ ;

    };

    class KeyItemContext {
    public:
        KeyItemContext(Builder& builder) : builder_(builder) {}
        DictItemContext& Value(Node node);
        DictItemContext& StartDict() ;
        ArrayItemContext& StartArray();
    private:
        Builder& builder_;
    };

    class Builder {
    public:
        Builder() : dict_item_context(*this), key_item_context(*this), array_item_context(*this){}
        Node Build() const;
        Builder& Value(Node node);
        DictItemContext& ValueDict(Node node);
        ArrayItemContext& ValueArray(Node node);
        KeyItemContext& Key(std::string key);
        DictItemContext& StartDict();
        Builder& EndDict();
        ArrayItemContext& StartArray();
        Builder& EndArray();
    private:
        DictItemContext dict_item_context;
        KeyItemContext key_item_context;
        ArrayItemContext array_item_context;
        bool flag_value = false;
        Node key_;
        Node root_;
        std::vector<Node*> nodes_stack_;
    };
}

