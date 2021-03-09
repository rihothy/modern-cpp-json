#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

enum class Type
{
    NONE,
    INT,
    DOUBLE,
    STR,
    VEC,
    MAP
};

class Ele
{
    using str = std::string;
    using vec = std::vector<std::shared_ptr<Ele>>;
    using map = std::unordered_map<std::string, std::shared_ptr<Ele>>;

    Type type = Type::NONE;

    int v_int = 0;
    double v_double = 0.0;
    str v_str;
    vec v_vec;
    map v_map;

private:

    static std::vector<str> split(const str& s)
    {
        int cnt1 = 0, cnt2 = 0;
        std::vector<str> words;
        bool is_str = false;
        str word;

        for (auto i = 0; i < s.length(); ++i)
        {
            if (s[i] == '"' && (!i || s[i] != '\\'))
            {
                is_str = !is_str;
                word += s[i];
            }
            else
            {
                if (is_str)
                {
                    word += s[i];
                }
                else
                {
                    switch (s[i])
                    {
                    case '[':
                        ++cnt1;
                        break;
                    case ']':
                        --cnt1;
                        break;
                    case '{':
                        ++cnt2;
                        break;
                    case '}':
                        --cnt2;
                        break;
                    case ',':
                    case ':':
                        if (!cnt1 && !cnt2)
                        {
                            words.push_back(word);
                            word = "";
                        }
                        break;
                    default:
                        break;
                    }

                    if ((cnt1 || cnt2) || s[i] != ',' && s[i] != ':')
                    {
                        word += s[i];
                    }
                }
            }
        }

        if (not word.empty())
        {
            words.push_back(word);
        }

        return words;
    }

public:

    Ele(void) = default;

    template<class T>
    Ele(const T v)
    {
        if constexpr (std::is_integral_v<T>)
        {
            type = Type::INT;
            v_int = v;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            type = Type::DOUBLE;
            v_double = v;
        }
        else if constexpr (std::is_same_v<T, str>)
        {
            type = Type::STR;
            v_str = v;
        }
        else if constexpr (std::is_same_v<T, vec>)
        {
            type = Type::VEC;
            v_vec = v;
        }
        else if constexpr (std::is_same_v<T, map>)
        {
            type = Type::MAP;
            v_map = v;
        }
        else if constexpr (std::is_same_v<T, Ele>)
        {
            type = v.type;
            v_int = v.v_int;
            v_double = v.v_double;
            v_str = v.v_str;
            v_vec = v.v_vec;
            v_map = v.v_map;
        }
    }

    Ele(const std::initializer_list<Ele>& list)
    {
        bool is_map = true;

        for (const auto& ele : list)
        {
            if (not (ele.type == Type::VEC && ele.size() == 2 && ele.v_vec[0]->type == Type::STR))
            {
                is_map = false;
                break;
            }
        }

        if (is_map)
        {
            type = Type::MAP;

            for (const auto& ele : list)
            {
                v_map.insert(std::make_pair(ele.v_vec[0]->v_str, std::make_shared<Ele>(*ele.v_vec[1])));
            }
        }
        else
        {
            type = Type::VEC;

            for (const auto& ele : list)
            {
                v_vec.push_back(std::make_shared<Ele>(ele));
            }
        }
    }

    operator int() const
    {
        switch (type)
        {
        case Type::INT:
            return v_int;
        case Type::DOUBLE:
            return int(v_double);
        case Type::STR:
            return std::stoi(v_str);
        default:
            throw std::bad_cast();
        }
    }

    operator double() const
    {
        switch (type)
        {
        case Type::INT:
            return double(v_int);
        case Type::DOUBLE:
            return v_double;
        case Type::STR:
            return std::stod(v_str);
        default:
            throw std::bad_cast();
        }
    }

    operator str() const
    {
        switch (type)
        {
        case Type::INT:
            return std::to_string(v_int);
        case Type::DOUBLE:
            return std::to_string(v_double);
        case Type::STR:
            return v_str;
        default:
            throw std::bad_cast();
        }
    }

    template<class T>
    auto& operator[](T v)
    {
        if constexpr (std::is_integral_v<T>)
        {
            if (type == Type::VEC || type == Type::NONE)
            {
                type = Type::VEC;

                if (v < 0)
                {
                    v = v_vec.size() + v;
                }

                if (v < 0)
                {
                    throw std::out_of_range("");
                }

                while (v_vec.size() <= v)
                {
                    v_vec.push_back(std::make_shared<Ele>());
                }

                return *v_vec[v];
            }
        }
        else if constexpr (std::is_same_v<T, str>)
        {
            if (type == Type::MAP || type == Type::NONE)
            {
                type = Type::MAP;

                if (not v_map.count(v))
                {
                    v_map.insert(std::make_pair(v, std::make_shared<Ele>()));
                }

                return *v_map[v];
            }
        }

        throw std::bad_cast();
    }

    size_t size(void) const
    {
        if (type == Type::VEC)
        {
            return v_vec.size();
        }
        else if (type == Type::MAP)
        {
            return v_map.size();
        }

        return 0;

        throw std::logic_error("");
    }

    friend std::ostream& operator<<(std::ostream& strm, const Ele& ele);
    friend std::string dumps(const Ele& ele, int indent, int cur_indent);
    friend Ele loads(const std::string& s, bool strip);
    friend Ele load(const std::string& path);
    friend void dump(const Ele& ele, const std::string& path, int indent);
};

std::ostream& operator<<(std::ostream& strm, const Ele& ele)
{
    return strm << dumps(ele, 0, 0);
}

std::string dumps(const Ele& ele, int indent = 0, int cur_indent = 0)
{
    std::string s;
    bool first = true;

    switch (ele.type)
    {
    case Type::NONE:
        s = "none";
        break;
    case Type::INT:
    case Type::DOUBLE:
        s = std::string(ele);
        break;
    case Type::STR:
        s = '"' + ele.v_str + '"';
        break;
    case Type::VEC:
        s += '[';

        for (const auto& ele : ele.v_vec)
        {
            s += (first ? "" : ", ") + dumps(*ele, indent, indent ? cur_indent : 0);
            first = false;
        }

        s += ']';
        break;
    case Type::MAP:
        auto now_indent = indent ? '\n' + std::string(cur_indent + indent, ' ') : "";

        s += '{';
        s += now_indent;

        for (const auto& [k, ele] : ele.v_map)
        {
            s += (first ? "" : ", " + now_indent) + '"' + k + "\": " + dumps(*ele, indent, indent ? cur_indent + indent : 0);
            first = false;
        }

        s += (indent ? "\n" + std::string(cur_indent, ' ') : "") + '}';
        break;
    }

    return s;
}

Ele loads(const std::string& s, bool strip = true)
{
    Ele ele;
    std::string temp;
    std::vector<std::string> words;

    if (strip)
    {
        for (const auto& ch : s)
        {
            if (ch != ' ' && ch != '\t' && ch != '\n')
            {
                temp += ch;
            }
        }
    }
    else
    {
        temp = s;
    }

    if (temp.empty())
    {
        return ele;
    }

    switch (temp[0])
    {
    case 'n':
        return Ele();
    case '"':
        return Ele(temp.substr(1, temp.length() - 2));
    case '[':
        ele.type = Type::VEC;
        temp = temp.substr(1, temp.length() - 2);
        words = Ele::split(temp);

        for (const auto& word : words)
        {
            ele.v_vec.push_back(std::make_shared<Ele>(loads(word, false)));
        }
        break;
    case '{':
        ele.type = Type::MAP;
        temp = temp.substr(1, temp.length() - 2);
        words = Ele::split(temp);

        for (int i = 0; i < words.size(); i += 2)
        {
            ele.v_map.insert(std::make_pair(words[i].substr(1, words[i].length() - 2), std::make_shared<Ele>(loads(words[i + 1], false))));
        }
        break;
    default:
        if (temp.find('.') != std::string::npos)
        {
            return Ele(std::stod(temp));
        }
        else
        {
            return Ele(std::stoi(temp));
        }
    }

    return ele;
}

void dump(const Ele& ele, const std::string& path, int indent = 0)
{
    std::ofstream ostrm(path);
    std::string s = dumps(ele, indent, 0);

    ostrm << s;
}

Ele load(const std::string& path)
{
    std::string s, line;
    std::ifstream istrm(path);

    while (std::getline(istrm, line))
    {
        s += line;
    }

    return loads(s);
}

using Json = Ele;

#endif