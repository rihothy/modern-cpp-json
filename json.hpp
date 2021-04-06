#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <initializer_list>
#include <unordered_map>
#include <stdexcept>
#include <variant>
#include <memory>
#include <vector>
#include <string>
#include <cctype>

#include "io.hpp"

class Json
{
	using null = void*;
	using str = std::string;
	using vec = std::vector<std::shared_ptr<Json>>;
	using dict = std::unordered_map<std::string, std::shared_ptr<Json>>;

	std::string raw_type = typeid(null).name();
	std::variant<null, bool, int, double, str, vec, dict> val = nullptr;

public:

	Json(void) = default;

	template<class T>
	Json(const T& v)
	{
		*this = v;
	}

	Json(const std::initializer_list<Json>& list)
	{
		auto is_dict = true;

		for (const auto& ele : list)
		{
			if (ele.type() != "vec" || ele.size() != 2 || ele[0].type() != "str")
			{
				is_dict = false;
				break;
			}
		}

		if (is_dict)
		{
			raw_type = typeid(dict).name();
			val = dict();

			for (const auto& ele : list)
			{
				std::get<dict>(val).insert(std::make_pair(str(ele[0]), std::make_shared<Json>(ele[1])));
			}
		}
		else
		{
			raw_type = typeid(vec).name();
			val = vec();

			for (const auto& ele : list)
			{
				std::get<vec>(val).push_back(std::make_shared<Json>(ele));
			}
		}
	}

	Json(const Json& another)
	{
		*this = another;
	}

	Json(Json&& another) noexcept
	{
		*this = std::move(another);
	}

	template<class T>
	auto& operator=(const T& v)
	{
		if constexpr (std::is_null_pointer_v<T>)
		{
			val = v;
			raw_type = typeid(null).name();
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			val = v;
			raw_type = typeid(bool).name();
		}
		else if constexpr (std::is_integral_v<T>)
		{
			val = int(v);
			raw_type = typeid(int).name();
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			val = double(v);
			raw_type = typeid(double).name();
		}
		else if constexpr (std::is_array_v<T> || std::is_pointer_v<T>)
		{
			*this = std::string(v);
		}
		else
		{
			val = v;
			raw_type = typeid(T).name();
		}

		return *this;
	}

	Json& operator=(const Json& another)
	{
		if (&another != this)
		{
			raw_type = another.raw_type;
			val = another.val;
		}

		return *this;
	}

	Json& operator=(Json&& another) noexcept
	{
		if (&another != this)
		{
			raw_type = another.raw_type;
			val = std::move(another.val);

			another.raw_type = typeid(null).name();
			another.val = nullptr;
		}

		return *this;
	}

	operator bool() const
	{
		if (type() == "null")
		{
			return false;
		}
		else if (type() == "bool")
		{
			return std::get<bool>(val);
		}
		else if (type() == "int")
		{
			return std::get<int>(val);
		}
		else if (type() == "double")
		{
			return std::get<double>(val);
		}
		else if (type() == "str")
		{
			return !std::get<str>(val).empty();
		}
		else if (type() == "vec")
		{
			return !std::get<vec>(val).empty();
		}
		else if (type() == "dict")
		{
			return !std::get<dict>(val).empty();
		}
	}

	operator int() const
	{
		if (type() == "null")
		{
			return 0;
		}
		else if (type() == "bool")
		{
			return std::get<bool>(val);
		}
		else if (type() == "int")
		{
			return std::get<int>(val);
		}
		else if (type() == "double")
		{
			return std::get<double>(val);
		}
		else if (type() == "str")
		{
			return std::stoi(std::get<str>(val));
		}
	}

	operator double() const
	{
		if (type() == "null")
		{
			return 0;
		}
		else if (type() == "bool")
		{
			return std::get<bool>(val);
		}
		else if (type() == "int")
		{
			return std::get<int>(val);
		}
		else if (type() == "double")
		{
			return std::get<double>(val);
		}
		else if (type() == "str")
		{
			return std::stod(std::get<str>(val));
		}
	}

	operator str() const
	{
		if (type() == "null")
		{
			return "null";
		}
		else if (type() == "bool")
		{
			return std::get<bool>(val) ? "true" : "false";
		}
		else if (type() == "int")
		{
			return std::to_string(std::get<int>(val));
		}
		else if (type() == "double")
		{
			return std::to_string(std::get<double>(val));
		}
		else if (type() == "str")
		{
			return std::get<str>(val);
		}
	}

	Json& operator[](int i)
	{
		if (i < 0)
		{
			i = this->size() + i;
		}

		return *std::get<vec>(val)[i];
	}

	Json operator[](int i) const
	{
		if (i < 0)
		{
			i = this->size() + i;
		}

		return *std::get<vec>(val)[i];
	}

	Json& operator[](const str& k)
	{
		return *std::get<dict>(val)[k];
	}

	Json operator[](const str& k) const
	{
		return *std::get<dict>(val).at(k);
	}

	size_t size(void) const
	{
		if (type() == "vec")
		{
			return std::get<vec>(val).size();
		}
		else if (type() == "dict")
		{
			return std::get<dict>(val).size();
		}
	}

	str type(void) const
	{
		if (raw_type == typeid(bool).name())
		{
			return "bool";
		}
		else if (raw_type == typeid(int).name())
		{
			return "int";
		}
		else if (raw_type == typeid(double).name())
		{
			return "double";
		}
		else if (raw_type == typeid(str).name())
		{
			return "str";
		}
		else if (raw_type == typeid(vec).name())
		{
			return "vec";
		}
		else if (raw_type == typeid(dict).name())
		{
			return "dict";
		}
		else
		{
			return "null";
		}
	}

	friend str dumps(const Json& json, int indent, int cur_indent);
	friend Json loads(const str& buff);
};

Json::str dumps(const Json& json, int indent = 0, int cur_indent = 0)
{
	Json::str buff;
	bool first = true;
	auto raw_indent = indent ? '\n' + std::string(cur_indent + indent, ' ') : "";

	auto str2str = [](const auto& str)
	{
		std::string temp = "\"";

		for (const auto& ch : str)
		{
			temp += ch == '"' ? "\\\"" : std::string(1, ch);
		}

		return temp + '"';
	};

	if (json.type() == "vec")
	{
		buff += "[" + raw_indent;

		for (int i = 0; i < json.size(); ++i)
		{
			buff += (i ? "," + raw_indent : "") + dumps(json[i], indent, cur_indent + indent);
		}

		buff += (indent ? '\n' + std::string(cur_indent, ' ') : "") + ']';
	}
	else if (json.type() == "dict")
	{
		buff += '{' + raw_indent;

		for (const auto& [k, ele] : std::get<Json::dict>(json.val))
		{
			buff += (first ? "" : "," + raw_indent) + str2str(k) + ": " + dumps(*ele, indent, cur_indent + indent);
			first = false;
		}

		buff += (indent ? '\n' + std::string(cur_indent, ' ') : "") + '}';
	}
	else if (json.type() == "str")
	{
		buff = str2str(Json::str(json));
	}
	else
	{
		buff = Json::str(json);
	}

	return buff;
}

Json loads(const Json::str& buff)
{
	int i = 0;

	auto skip = [&](const auto&... chs) -> void
	{
		while (((buff[i] == chs) || ...))
		{
			++i;
		}
	};

	auto run = [&](auto&& self) -> Json
	{
		skip(' ', '\n', '\t');

		if (buff[i] == 'n')
		{
			i += 4;
			return Json(nullptr);
		}
		else if (buff[i] == 't')
		{
			i += 4;
			return Json(true);
		}
		else if (buff[i] == 'f')
		{
			i += 5;
			return Json(false);
		}
		else if (buff[i] == '"')
		{
			Json::str temp;

			while (buff[++i] != '"' || !++i)
			{
				if (buff[i] == '\\')
				{
					++i;
				}

				temp += buff[i];
			}

			return Json(temp);
		}
		else if (buff[i] == '[')
		{
			Json json;

			json.raw_type = typeid(Json::vec).name();
			json.val = Json::vec();
			skip(' ', '\n', '\t', '[');

			while (buff[i] != ']' || !++i)
			{
				auto ele = self(self);

				std::get<Json::vec>(json.val).push_back(std::make_shared<Json>(ele));
				skip(' ', '\n', '\t', ',');
			}

			return json;
		}
		else if (buff[i] == '{')
		{
			Json json;

			json.raw_type = typeid(Json::dict).name();
			json.val = Json::dict();
			skip(' ', '\n', '\t', '{');

			while (buff[i] != '}' || !++i)
			{
				auto key = self(self);
				skip(' ', '\n', '\t', ':');
				auto ele = self(self);
				skip(' ', '\n', '\t', ',');

				std::get<Json::dict>(json.val).insert(std::make_pair(Json::str(key), std::make_shared<Json>(ele)));
			}

			return json;
		}
		else
		{
			Json::str temp;
			bool is_float = false;

			while (true)
			{
				if (isdigit(buff[i]) || buff[i] == '.')
				{
					temp += buff[i];
				}
				else
				{
					break;
				}

				is_float = is_float || buff[i] == '.';
				++i;
			}

			if (is_float)
			{
				return Json(std::stod(temp));
			}
			else
			{
				return Json(std::stoi(temp));
			}
		}
	};

	return run(run);
}

#endif