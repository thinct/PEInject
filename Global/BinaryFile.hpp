#ifndef __BINARY_FILE_HPP__
#define __BINARY_FILE_HPP__

#include <fstream>
#include <string>
#include <vector>

namespace FileSystem
{
    class BinaryFile
    {
    public:
        BinaryFile() = default;

        BinaryFile(const std::string& filePath, std::ios_base::openmode mode)
        {
            open(filePath, mode);
        }

        bool open(const std::string& filePath, std::ios_base::openmode mode)
        {
            m_stream.open(filePath, mode | std::fstream::binary);
            if (!m_stream.is_open())
            {
                return false;
            }

            seek(0, std::ios_base::end);
            m_size = tell();
            seek(0);

            return true;
        }

        void flush()
        {
            m_stream.flush();
        }

        void close()
        {
            m_stream.close();
        }

        void seek(std::size_t off, std::ios_base::seekdir dir = std::ios_base::beg)
        {
            m_stream.seekg(off, dir);
        }

        std::size_t tell()
        {
            return m_stream.tellg();
        }

        template <typename T>
        void read(T& t)
        {
            if (tell() + sizeof(T) > m_size)
            {
                throw std::runtime_error("File end reached.");
            }

            m_stream.read(reinterpret_cast<char*>(&t), sizeof(T));
        }

        template <typename T>
        T read()
        {
            T result;

            read(result);

            return result;
        }

        std::vector<unsigned char> readBuffer(std::size_t size)
        {
            if (tell() + size > m_size)
            {
                throw std::runtime_error("File end reached.");
            }

            std::vector<unsigned char> buffer;
            buffer.resize(size);

            m_stream.read(reinterpret_cast<char*>(buffer.data()), size);

            return buffer;
        }


        template <typename T>
        void write(T& t)
        {
            m_stream.write(reinterpret_cast<char*>(&t), sizeof(T));
            updateSize();
        }

        template <typename T>
        T write()
        {
            T result;
            write(result);
            updateSize();
            return result;
        }

        void writeBuffer(std::vector<unsigned char> buffer)
        {
            m_stream.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
            updateSize();
        }

        const std::size_t& size() const
        {
            return m_size;
        }

    private:
        void updateSize()
        {
            seek(0, std::ios_base::end);
            m_size = tell();
        }

    private:
        std::string  m_strFilePath;
        std::size_t  m_size;
        std::fstream m_stream;
    };
}

#endif // __BINARY_FILE_HPP__

