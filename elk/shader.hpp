#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

class Shader
{
public:
    const GLuint ID;
    Shader(const char* vert_path, const char* frag_path) : ID(glCreateProgram()) {
        std::string vert_buf;
        std::string frag_buf;
        std::ifstream vert_file;
        std::ifstream frag_file;
        // ensure ifstream objects can throw exceptions:
        vert_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        frag_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            vert_file.open(vert_path);
            frag_file.open(frag_path);
            std::stringstream vert_stream, frag_stream;

            vert_stream << vert_file.rdbuf();
            frag_stream << frag_file.rdbuf();

            vert_file.close();
            frag_file.close();

            vert_buf = vert_stream.str();
            frag_buf = frag_stream.str();
        }
        catch (std::ifstream::failure& e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }

        const char* vert_code = vert_buf.c_str();
        const char* frag_code = frag_buf.c_str();

        GLuint vertex, fragment;

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vert_code, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &frag_code, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void use() const {
        glUseProgram(ID);
    }

    void setBool(const std::string& name, bool value) {
        setInt(name, value);
    }

    void setInt(const std::string& name, int value) {
        auto loc = lut.find(name);
        if (loc != lut.end()) {
            glUniform1i(loc->second, value);
        }
        else {
            GLuint new_loc = glGetUniformLocation(ID, name.c_str());
            lut[name] = new_loc;
            glUniform1i(new_loc, value);
        }
    }

    void setFloat(const std::string& name, float value) {
        auto loc = lut.find(name);
        if (loc != lut.end()) {
            glUniform1f(loc->second, value);
        }
        else {
            GLuint new_loc = glGetUniformLocation(ID, name.c_str());
            lut[name] = new_loc;
            glUniform1f(new_loc, value);
        }
    }

    void setVec2(const std::string& name, const glm::vec2& value)
    {
        auto loc = lut.find(name);
        if (loc != lut.end()) {
            glUniform2fv(loc->second, 1, &value[0]);
        }
        else {
            GLuint new_loc = glGetUniformLocation(ID, name.c_str());
            lut[name] = new_loc;
            glUniform2fv(new_loc, 1, &value[0]);
        }
    }
    void setVec2(const std::string& name, float x, float y)
    {
        auto loc = lut.find(name);
        if (loc != lut.end()) {
            glUniform2f(loc->second, x, y);
        }
        else {
            GLuint new_loc = glGetUniformLocation(ID, name.c_str());
            lut[name] = new_loc;
            glUniform2f(new_loc, x, y);
        }
    }

    void setVec3(const std::string& name, const glm::vec3& value)
    {
        auto loc = lut.find(name);
        if (loc != lut.end()) {
            glUniform3fv(loc->second, 1, &value[0]);
        }
        else {
            GLuint new_loc = glGetUniformLocation(ID, name.c_str());
            lut[name] = new_loc;
            glUniform3fv(new_loc, 1, &value[0]);
        }
    }
    void setVec3(const std::string& name, float x, float y, float z)
    {
        auto loc = lut.find(name);
        if (loc != lut.end()) {
            glUniform3f(loc->second, x, y, z);
        }
        else {
            GLuint new_loc = glGetUniformLocation(ID, name.c_str());
            lut[name] = new_loc;
            glUniform3f(new_loc, x, y, z);
        }
    }

    void setVec4(const std::string& name, const glm::vec4& value)
    {
        auto loc = lut.find(name);
        if (loc != lut.end()) {
            glUniform4fv(loc->second, 1, &value[0]);
        }
        else {
            GLuint new_loc = glGetUniformLocation(ID, name.c_str());
            lut[name] = new_loc;
            glUniform4fv(new_loc, 1, &value[0]);
        }
    }
    void setVec4(const std::string& name, float x, float y, float z, float w)
    {
        auto loc = lut.find(name);
        if (loc != lut.end()) {
            glUniform4f(loc->second, x, y, z, w);
        }
        else {
            GLuint new_loc = glGetUniformLocation(ID, name.c_str());
            lut[name] = new_loc;
            glUniform4f(new_loc, x, y, z, w);
        }
    }

    void setMat2(const std::string& name, const glm::mat2& mat)
    {
        auto loc = lut.find(name);
        if (loc != lut.end()) {
            glUniformMatrix2fv(loc->second, 1, GL_FALSE, &mat[0][0]);
        }
        else {
            GLuint new_loc = glGetUniformLocation(ID, name.c_str());
            lut[name] = new_loc;
            glUniformMatrix2fv(new_loc, 1, GL_FALSE, &mat[0][0]);
        }
    }

    void setMat3(const std::string& name, const glm::mat3& mat)
    {
        auto loc = lut.find(name);
        if (loc != lut.end()) {
            glUniformMatrix3fv(loc->second, 1, GL_FALSE, &mat[0][0]);
        }
        else {
            GLuint new_loc = glGetUniformLocation(ID, name.c_str());
            lut[name] = new_loc;
            glUniformMatrix3fv(new_loc, 1, GL_FALSE, &mat[0][0]);
        }
    }

    void setMat4(const std::string& name, const glm::mat4& mat)
    {
        auto loc = lut.find(name);
        if (loc != lut.end()) {
            glUniformMatrix4fv(loc->second, 1, GL_FALSE, &mat[0][0]);
        }
        else {
            GLuint new_loc = glGetUniformLocation(ID, name.c_str());
            lut[name] = new_loc;
            glUniformMatrix4fv(new_loc, 1, GL_FALSE, &mat[0][0]);
        }
    }

private:
    std::map<const std::string, GLuint> lut;

    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

#endif