#pragma once
// JsonValue: minimal JSON parser implemented to avoid external dependencies
// Supports only the subset needed for config.json: objects, arrays
// strings, numbers, booleans, and null. Not intended as a production-grade
// parser (no unicode escapes, no comments, limited error handling)

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <cctype>
#include <fstream>
#include <sstream>

class JsonValue {
public:
	enum class Type { Null, Bool, Number, String, Array, Object };

	JsonValue() : type_(Type::Null) {}

	Type type() const { return type_; }

	// --- Accessors with default values for optional fields ---
	bool asBool(bool def = false) const { return type_ == Type::Bool ? bool_ : def; }
	double asNumber(double def = 0.0) const { return type_ == Type::Number ? number_ : def; }
	int asInt(int def = 0) const { return type_ == Type::Number ? (int)number_ : def; }
	std::string asString(const std::string& def = "") const { return type_ == Type::String ? string_ : def; }

	const std::vector<JsonValue>& asArray() const {
		static std::vector<JsonValue> empty;
		return type_ == Type::Array ? array_ : empty;
	}

	// Access a field of an object. If the key does not exist or the value
	// is not an object, returns a Null JsonValue. This allows chained access
	const JsonValue& operator[](const std::string& key) const {
		static JsonValue null_value;
		if (type_ != Type::Object) return null_value;
		auto it = object_.find(key);
		return it != object_.end() ? it->second : null_value;
	}

	bool has(const std::string& key) const {
		return type_ == Type::Object && object_.find(key) != object_.end();
	}

	// Parses any JSON value based on the first character
	static JsonValue parseFile(const std::string& path) {
		std::ifstream f(path);
		if (!f) throw std::runtime_error("Impossibile aprire il file di config: " + path);
		std::stringstream ss;
		ss << f.rdbuf();
		return parse(ss.str());
	}

	static JsonValue parse(const std::string& text) {
		size_t pos = 0;
		JsonValue v = parseValue(text, pos);
		return v;
	}

private:
	Type type_;
	bool bool_ = false;
	double number_ = 0.0;
	std::string string_;
	std::vector<JsonValue> array_;
	std::map<std::string, JsonValue> object_;

	static void skipWs(const std::string& s, size_t& i) {
		while (i < s.size() && std::isspace((unsigned char)s[i])) i++;
	}

	static JsonValue parseValue(const std::string& s, size_t& i) {
		skipWs(s, i);
		if (i >= s.size()) throw std::runtime_error("JSON troncato");

		char c = s[i];
		if (c == '{') return parseObject(s, i);
		if (c == '[') return parseArray(s, i);
		if (c == '"') return parseString(s, i);
		if (c == 't' || c == 'f') return parseBool(s, i);
		if (c == 'n') { i += 4; return JsonValue(); } // "null"
		return parseNumber(s, i);
	}

	static JsonValue parseObject(const std::string& s, size_t& i) {
		JsonValue v; v.type_ = Type::Object;
		i++; // {
		skipWs(s, i);
		if (i < s.size() && s[i] == '}') { i++; return v; }
		while (true) {
			skipWs(s, i);
			JsonValue key = parseString(s, i);
			skipWs(s, i);
			if (i >= s.size() || s[i] != ':') throw std::runtime_error("Atteso ':' nel JSON");
			i++; // :
			JsonValue val = parseValue(s, i);
			v.object_[key.string_] = val;
			skipWs(s, i);
			if (i < s.size() && s[i] == ',') { i++; continue; }
			if (i < s.size() && s[i] == '}') { i++; break; }
			throw std::runtime_error("Atteso ',' o '}' nel JSON");
		}
		return v;
	}

	static JsonValue parseArray(const std::string& s, size_t& i) {
		JsonValue v; v.type_ = Type::Array;
		i++; // [
		skipWs(s, i);
		if (i < s.size() && s[i] == ']') { i++; return v; }
		while (true) {
			JsonValue val = parseValue(s, i);
			v.array_.push_back(val);
			skipWs(s, i);
			if (i < s.size() && s[i] == ',') { i++; continue; }
			if (i < s.size() && s[i] == ']') { i++; break; }
			throw std::runtime_error("Atteso ',' o ']' nel JSON");
		}
		return v;
	}

	static JsonValue parseString(const std::string& s, size_t& i) {
		JsonValue v; v.type_ = Type::String;
		if (s[i] != '"') throw std::runtime_error("Attesa stringa nel JSON");
		i++; // "
		std::string out;
		while (i < s.size() && s[i] != '"') {
			char c = s[i];
			if (c == '\\' && i + 1 < s.size()) {
				char n = s[i + 1];
				switch (n) {
					case 'n': out += '\n'; break;
					case 't': out += '\t'; break;
					case 'r': out += '\r'; break;
					case '"': out += '"'; break;
					case '\\': out += '\\'; break;
					case '/': out += '/'; break;
					default: out += n; break;
				}
				i += 2;
			} else {
				out += c;
				i++;
			}
		}
		i++; // "
		v.string_ = out;
		return v;
	}

	static JsonValue parseBool(const std::string& s, size_t& i) {
		JsonValue v; v.type_ = Type::Bool;
		if (s.compare(i, 4, "true") == 0) { v.bool_ = true; i += 4; }
		else if (s.compare(i, 5, "false") == 0) { v.bool_ = false; i += 5; }
		else throw std::runtime_error("Atteso booleano nel JSON");
		return v;
	}

	// Parses integer or floating-point numbers
	static JsonValue parseNumber(const std::string& s, size_t& i) {
		JsonValue v; v.type_ = Type::Number;
		size_t start = i;
		while (i < s.size() && (std::isdigit((unsigned char)s[i]) || s[i] == '-' || s[i] == '+' ||
				s[i] == '.' || s[i] == 'e' || s[i] == 'E')) i++;
		v.number_ = std::stod(s.substr(start, i - start));
		return v;
	}
};
