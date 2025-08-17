#pragma once

#pragma once

#include <chrono>
#include <memory>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace
{
    struct normal_tag;
    struct enum_tag;
    struct chrono_tag;

    template <typename T>
    struct config_item_traits
    {
        using tag = std::conditional_t<std::is_enum_v<T>, enum_tag, normal_tag>;
    };

    template<typename rep, typename period>
    struct config_item_traits<std::chrono::duration<rep, period>>
    {
        using tag = chrono_tag;
    };

    template <typename type>
    using config_item_tag = typename config_item_traits<type>::tag;

    template<typename type, typename tag = config_item_tag<type>>
    class config_item;

    template<typename type, typename source_type>
    class config_item_base
    {
    public:
        using source_t = source_type;

        config_item_base(const char* name, type& link, bool hide)
            : m_name(name)
            , m_ptr(link)
            , m_default_value()
            , m_hide(hide)
        {
        }

        const char* get_name() const { return m_name; }
        bool has_default_value() const { return m_default_value.is_initialized(); }
        const source_t& get_default_value() const { return *m_default_value; }
        bool is_hide() const { return m_hide; }

        const typename source_t& get() const { return m_ptr; }
        void set(const typename source_t& value) { m_ptr = value; }

    protected:
        void set_default_value(const type& default_value)
        {
            m_default_value = default_value;
        }

    private:
        const char* m_name;
        type& m_ptr;
        boost::optional<source_t>  m_default_value;
        bool            m_hide;
    };

    template<typename type>
    class config_item<type, normal_tag> : public config_item_base<type, type>
    {
        using base_t = config_item_base<type, type>;
    public:
        using base_t::base_t;

        config_item<type>&& operator [] (const type& default_value)&&
        {
            base_t::set_default_value(default_value);
            return std::move(*this);
        }
    };

    template<typename type>
    class config_item<type, enum_tag> : public config_item_base<type, std::underlying_type_t<type>>
    {
        using base_t = config_item_base<type, type>;
    public:
        using base_t::base_t;

        config_item<type>&& operator [] (const type& default_value)&&
        {
            base_t::set_default_value(static_cast<typename base_t::source_t>(default_value));
            return std::move(*this);
        }
    };

    template<typename type>
    class config_item<type, chrono_tag> : public config_item_base<type, typename type::rep>
    {
        using base_t = config_item_base<type, type>;
    public:
        using base_t::base_t;

        template <typename chrono_type>
        config_item<type>&& operator [] (const chrono_type& default_value)&&
        {
            base_t::set_default_value(type{ default_value }.count());
            return std::move(*this);
        }
    };

    //////////////////////////////////////////////////////////////////////////

    template<typename base_class>
    class ptree_ini_base_archive
    {
    public:
        template<typename type>
        static inline config_item<type> make_item(const char* name, type& link, bool hide = false)
        {
            return config_item<type>(name, link, hide);
        }

    protected:
        template<typename type>
        inline std::string make_name(const config_item<type>& item) const
        {
            return m_group_name + "." + item.get_name();
        }

        inline bool set_group_name(const std::string& group_name)
        {
            if (m_group_name.empty() && !group_name.empty())
            {
                m_group_name = group_name;
                return true;
            }
            return false;
        }

        inline void check_group_name() const
        {
            if (m_group_name.empty())
            {
                throw std::runtime_error("Group name is undefined for ini config file, function: " FUNCTION);
            }
        }

        std::string    m_group_name;
    };

    class ptree_ini_iarchive : public ptree_ini_base_archive<ptree_ini_iarchive>
    {
    public:
        ptree_ini_iarchive(boost::property_tree::ptree& tree)
            : m_tree(tree)
            , m_has_values(false)
        {
        }
        ptree_ini_iarchive& operator () (const std::string& group_name)
        {
            set_group_name(group_name);
            return *this;
        }

        template<typename type>
        ptree_ini_iarchive& operator & (config_item<type>&& item)
        {
            using source_t = typename config_item<type>::source_t;

            check_group_name();

            const auto name = make_name(item);
            m_has_values |= (bool)m_tree.get_child_optional(name);

            const auto value = item.has_default_value() ?
                m_tree.get<source_t>(name, item.get_default_value()) :
                m_tree.get<source_t>(name);

            item.set(value);
            return *this;
        }

        inline bool has_values() const { return m_has_values; }

    private:
        bool              m_has_values;
        boost::property_tree::ptree& m_tree;
    };

    class ptree_ini_oarchive : public ptree_ini_base_archive<ptree_ini_iarchive>
    {
    public:
        ptree_ini_oarchive(boost::property_tree::ptree& tree)
            : m_tree(tree)
        {
        }

        ptree_ini_oarchive& operator () (const std::string& group_name)
        {
            set_group_name(group_name);
            return *this;
        }

        template<typename type>
        ptree_ini_oarchive& operator & (config_item<type>&& item)
        {
            check_group_name();

            m_tree.put(make_name(item), item.get());
            return *this;
        }

    private:
        boost::property_tree::ptree& m_tree;
    };

    class ptree_ini_sarchive : public ptree_ini_base_archive<ptree_ini_iarchive>
    {
    public:
        ptree_ini_sarchive(std::ostream& stream)
            : m_stream(stream)
        {
        }

        ptree_ini_sarchive& operator () (const std::string& group_name)
        {
            if (set_group_name(group_name))
            {
                m_stream << '[' << m_group_name << ']' << '\t';
            }
            return *this;
        }

        template<typename type>
        ptree_ini_sarchive& operator & (config_item<type>&& item)
        {
            check_group_name();

            m_stream << item.get_name() << '=';
            if (item.is_hide())
            {
                m_stream << "******";
            }
            else
            {
                m_stream << item.get();
            }
            m_stream << '\t';
            return *this;
        }

    private:
        std::ostream& m_stream;
    };

    struct initialize_archive
    {
        initialize_archive& operator () (const std::string&) { return *this; }

        template<typename type>
        initialize_archive& operator & (config_item<type>&& item)
        {
            item.set(item.has_default_value() ? item.get_default_value() : type{});
            return *this;
        }
    };
}

namespace ini
{
    template<typename type>
    bool load_ini(const std::string& file_path, const std::string& group_name, type& data_type)
    {
        boost::property_tree::ptree property_tree;

        std::ifstream file_stream(file_path);
        boost::property_tree::read_ini(file_stream, property_tree);

        ::ptree_ini_iarchive ar(property_tree);
        ar(group_name);
        ::serialize(ar, data_type);

        return ar.has_values();
    }

    template<typename type>
    bool load_ini(const std::string& file_path, type& data_type)
    {
        return load_ini<type>(file_path, {}, data_type);
    }

    template<typename type>
    type load_ini(const std::string& file_path)
    {
        type data_type{};
        load_ini<type>(file_path, {}, data_type);
        return data_type;
    }

    template<typename type>
    void save_ini(const std::string& file_path, const std::string& group_name, type& data_type)
    {
        boost::property_tree::ptree property_tree;
        {
            std::ifstream file_stream(file_path);
            boost::property_tree::read_ini(file_stream, property_tree);
        }

        ::ptree_ini_oarchive ar(property_tree);
        ar(group_name);
        ::serialize(ar, data_type);

        std::ofstream file_stream(file_path);
        boost::property_tree::write_ini(file_stream, property_tree);
    }
    template<typename type>
    void save_ini(const std::string& file_path, type& data_type)
    {
        save_ini<type>(file_path, {}, data_type);
    }
    template<typename type>
    void print_ini(std::ostream& stream, const std::string& group_name, type& data_type)
    {
        ::ptree_ini_sarchive ar(stream);
        ar(group_name);
        ::serialize(ar, data_type);
    }
    template<typename type>
    void print_ini(std::ostream& stream, type& data_type)
    {
        print_ini<type>(stream, {}, data_type);
    }

    template<typename type>
    std::string print_ini(const std::string& group_name, type& data_type)
    {
        std::stringstream str;
        print_ini(str, group_name, data_type);
        return str.str();
    }
    template<typename type>
    std::string print_ini(type& data_type)
    {
        std::stringstream str;
        print_ini<type>(str, {}, data_type);
        return str.str();
    }

    template<typename type>
    void initialize_ini(type& data_type)
    {
        ::serialize(initialize_archive{}, data_type);
    }
}