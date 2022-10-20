#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "LightBase.h"
#include "Camera.h"
#include "Texture.h"
#include "Transformation.h"
#include <vector>
#include <map>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
    using string = std::string;
    using ifstream = std::ifstream;
    using vec3 = glm::vec3;
    using mat4 = glm::mat4;
private:
    int materialTextureUnit = 0;    // 0 - 6 textures: diff/base, specular/metalic, gloss/roughness, height, normal, ao...
    int cubemapTextureUnit = 15;     // 7 - 9 textures: 7 = irradiance map, 8 = pre-filter cubemap, 9 = BRDF LUT
    int shadowMapTextureUnit = 18;  // 10 - 32 textures: / 10 direct / 11 - 20 spot / 21 - 32 point /

    string shaderFolder = "Shaders/";

    // Utility function for checking shader compilation/linking errors.
    void checkCompileErrors(unsigned int shader, string type)
    {
        int success;
        char infoLog[1024];
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

    // Shader with the possibility to change the #define
    void createModifiedShader(const char* path, std::map<string, const char*> defineMod, GLenum shaderType)
    {
        string code = readFile(path);

        std::map<string, const char*>::iterator itr;
        for (itr = defineMod.begin(); itr != defineMod.end(); itr++)
        {
            setDefine(code, itr->first, itr->second);
        }

        unsigned int shader = createShader(code, shaderType);
        attachShader(shader);
        deleteShader(shader);
    }

public:
    // Program ID
    unsigned int ID;

    // Empty shader. Manually create shader, compile attach to a program, etc
    Shader()
    {

    }

    ~Shader() {}

    // Shader program with the possibility to change the #define values
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath, std::map<string, const char*> defineMod)
    {
        // If a static shader initializes before main(), load GL functions now
        //OpenGLWrapper::loadGLFunctions();

        // Shader Program
        ID = glCreateProgram();

        // Vertex
        if (vertexPath != "") createModifiedShader(vertexPath, defineMod, GL_VERTEX_SHADER);

        // Fragment
        if (fragmentPath != "") createModifiedShader(fragmentPath, defineMod, GL_FRAGMENT_SHADER);

        // Geometry
        if (geometryPath != "") createModifiedShader(geometryPath, defineMod, GL_GEOMETRY_SHADER);

        compileProgram();
        
    }

    string readFile(const char* filePath)
    {
        string shaderCode;
        ifstream shaderFile;
        shaderFile.exceptions(ifstream::failbit | ifstream::badbit);

        try
        {
            // Open files
            shaderFile.open(shaderFolder + filePath);

            std::stringstream shaderStream;
            // Read file's buffer contents into streams
            shaderStream << shaderFile.rdbuf();

            // Close files
            shaderFile.close();

            // Convert stream into string
            shaderCode = shaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }

        return shaderCode;
    }

    void setDefine(string &code, string defineName, const char* defineValue)
    {
        size_t definePos = code.find("#define " + defineName);

        if (definePos == string::npos)
        {
            std::cout << "ERROR::SHADER::#define " << defineName << " not found." << std::endl;
            return;
        }

        size_t definePosOffset = definePos;

        for (definePosOffset; definePosOffset < code.length(); ++definePosOffset)
        {
            if (code[definePosOffset] == '\n')
                break;
        }

        code = code.substr(0, definePos)+ "#define " + defineName + " " + defineValue + code.substr(definePosOffset);
    }

    unsigned int createShader(string& shaderCode, GLenum shaderType)
    {
        unsigned int shader = glCreateShader(shaderType);
        const char* shaderC = shaderCode.c_str();
        glShaderSource(shader, 1, &shaderC, NULL);
        glCompileShader(shader);
        switch (shaderType)
        {
        case GL_VERTEX_SHADER:
            checkCompileErrors(shader, "VERTEX");
        case GL_FRAGMENT_SHADER:
            checkCompileErrors(shader, "FRAGMENT");
        case GL_GEOMETRY_SHADER:
            checkCompileErrors(shader, "GEOMETRY");
        default:
            break;
        }

        return shader;
    }

    void attachShader(unsigned int shader)
    {
        glAttachShader(ID, shader);
    }

    void compileProgram()
    {
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
    }

    void deleteShader(unsigned int shader)
    {
        glDeleteShader(shader);
    }

    // This constructor reads vertex and fragment shader source code from 2 files and create a program (optionally can read geometry shader)
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = "")
    {
        // If a static shader initializes before main(), load GL functions now
        //OpenGLWrapper::loadGLFunctions();

        bool useGeometry = false;
        if (geometryPath != "") useGeometry = true;

        // Source code in a string
        string vertexCode;
        string fragmentCode;
        string geometryCode;

        // Source code file
        ifstream vShaderFile;
        ifstream fShaderFile;
        ifstream gShaderFile;

        // Ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
        fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
        gShaderFile.exceptions(ifstream::failbit | ifstream::badbit);

        try
        {
            // Open files
            vShaderFile.open(shaderFolder + vertexPath);
            fShaderFile.open(shaderFolder + fragmentPath);

            std::stringstream vShaderStream, fShaderStream;
            // Read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            // Close files
            vShaderFile.close();
            fShaderFile.close();

            // Convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();

            if (useGeometry)
            {
                std::stringstream gShaderStream;
                gShaderFile.open(shaderFolder + geometryPath);
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();

            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        unsigned int vertex, fragment, geometry;

        // Vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        //glShaderSourcePrint(vertex, 1, &vShaderCode, NULL); // Testing...
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        //glShaderSourcePrint(fragment, 1, &fShaderCode, NULL); // Testing...
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        // Geometry Shader
        if (useGeometry)
        {
            const char* gShaderCode = geometryCode.c_str();

            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }

        // Shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (useGeometry) glAttachShader(ID, geometry);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        // Delete the shaders as they're linked into our program and now no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (useGeometry) glDeleteShader(geometry);
    }

    // Activate shader
    void use()
    {
        glUseProgram(ID);
    }

    void addCubemapLight(unsigned int irradianceMap, unsigned int prefilterMap, unsigned int brdfLut)
    {
        glActiveTexture(GL_TEXTURE0 + cubemapTextureUnit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        setInt("irradianceMap", cubemapTextureUnit);

        glActiveTexture(GL_TEXTURE0 + cubemapTextureUnit + 1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        setInt("prefilterMap", cubemapTextureUnit + 1);

        glActiveTexture(GL_TEXTURE0 + cubemapTextureUnit + 2);
        glBindTexture(GL_TEXTURE_2D, brdfLut);
        setInt("brdfLUT", cubemapTextureUnit + 2);

        glActiveTexture(GL_TEXTURE0);

    }

    void addDirectionalLight(DirectionalLight dirLight)
    {
        setVec3("dirlight.color", dirLight.color);
        setVec3("dirlight.direction", dirLight.getDirection());

        setFloat("dirlight.ambient", dirLight.ambient);
        setFloat("dirlight.diffuse", dirLight.diffuse);
        setFloat("dirlight.specular", dirLight.specular);

        // Directional light shadow map
        glActiveTexture(GL_TEXTURE0 + shadowMapTextureUnit);
        glBindTexture(GL_TEXTURE_2D, dirLight.shadowMap);
        setInt("dShadowMap", shadowMapTextureUnit);
        glActiveTexture(GL_TEXTURE0);

        mat4 lightProjection = dirLight.lightCamera->getProjectionMatrix(dirLight.perspective);
        mat4 lightView = dirLight.lightCamera->getViewMatrix();
        mat4 lightSpaceMatrix = lightProjection * lightView;

        setMat4("dlightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));
    }

    void addSpotLight(std::vector<SpotLight> sLight)
    {
        int spShadowMapTexUnit = shadowMapTextureUnit + 1;

        for (int i = 0; i < sLight.size(); i++) 
        {
            string sl = "spotLight[" + std::to_string(i);
            
            setVec3(sl + "].color", sLight[i].color);
            setVec3(sl + "].pos", sLight[i].getPosition());
            setVec3(sl + "].direction", sLight[i].getDirection());
            setFloat(sl + "].cutOff", glm::cos(glm::radians(sLight[i].getCutOff())));
            setFloat(sl + "].oCutOff", glm::cos(glm::radians(sLight[i].getOuterCutOff())));
            //setFloat(sl + "].cutOff", glm::cos(glm::radians(12.5f)));

            setFloat(sl + "].ambient", sLight[i].ambient);
            setFloat(sl + "].diffuse", sLight[i].diffuse);
            setFloat(sl + "].specular", sLight[i].specular);

            setFloat(sl + "].constant", sLight[i].constant);
            setFloat(sl + "].linear", sLight[i].linear);
            setFloat(sl + "].quadratic", sLight[i].quadratic);

            string sm = "sShadowMap[" + std::to_string(i);

            glActiveTexture(GL_TEXTURE0 + spShadowMapTexUnit + i);
            glBindTexture(GL_TEXTURE_2D, sLight[i].shadowMap);
            setInt(sm + "]", spShadowMapTexUnit + i);
            glActiveTexture(GL_TEXTURE0);

            mat4 lightProjection = sLight[i].lightCamera->getProjectionMatrix(sLight[i].perspective);
            mat4 lightView = sLight[i].lightCamera->getViewMatrix();
            mat4 lightSpaceMatrix = lightProjection * lightView;

            string slsm = "slightSpaceMatrix[" + std::to_string(i);
            setMat4(slsm + "]", glm::value_ptr(lightSpaceMatrix));
        }
        
    }

    void addPointLight(std::vector<PointLight> pLight)
    {
        int spShadowMapTexUnit = shadowMapTextureUnit + 11;

        for (int i = 0; i < pLight.size(); i++)
        {
            string sl = "pointLight[" + std::to_string(i);

            setVec3(sl + "].color", pLight[i].color);
            setVec3(sl + "].pos", pLight[i].getPosition());

            setFloat(sl + "].ambient", pLight[i].ambient);
            setFloat(sl + "].diffuse", pLight[i].diffuse);
            setFloat(sl + "].specular", pLight[i].specular);

            setFloat(sl + "].constant", pLight[i].constant);
            setFloat(sl + "].linear", pLight[i].linear);
            setFloat(sl + "].quadratic", pLight[i].quadratic);

            setFloat(sl + "].farPlane", pLight[i].lightCamera->getFarPlane());

            string sm = "pShadowMap[" + std::to_string(i);

            glActiveTexture(GL_TEXTURE0 + spShadowMapTexUnit + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, pLight[i].shadowMap);
            setInt(sm + "]", spShadowMapTexUnit + i);
            glActiveTexture(GL_TEXTURE0);
        }

    }

    void addCamera(const Camera& cam)
    {
        mat4 view = cam.getViewMatrix();
        mat4 projection = cam.getProjectionMatrix(true);

        setVec3("cameraPos", cam.getPosition());
        setMat4("view", glm::value_ptr(view));
        setMat4("projection", glm::value_ptr(projection));

    }

    void setTextures(std::vector<Texture> tex)
    {
        unsigned int diffuseNr = 1;
        unsigned int baseNr = 1;
        unsigned int specularNr = 1;
        unsigned int metallicNr = 1;
        unsigned int normalNr = 1;
        unsigned int depthNr = 1;
        unsigned int roughnessNr = 1;
        unsigned int aoNr = 1;
        unsigned int opacityNr = 1;

        if(tex.size()>9)
            std::cout << "WARNING::SHADER_TEXTURE_OVERFLOW" << std::endl;

        for (unsigned int i = 0; i < tex.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + materialTextureUnit + i);
            glBindTexture(GL_TEXTURE_2D, tex[i].id);

            // retrieve texture number (the N in diffuse_textureN)
            string number;
            string name = tex[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_base")
                number = std::to_string(baseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);
            else if (name == "texture_metallic")
                number = std::to_string(metallicNr++);
            else if (name == "texture_normal")
                number = std::to_string(normalNr++);
            else if (name == "texture_depth")
                number = std::to_string(depthNr++);
            else if (name == "texture_roughness")
                number = std::to_string(roughnessNr++);
            else if (name == "texture_ao")
                number = std::to_string(aoNr++);
            else if (name == "texture_opacity")
                number = std::to_string(opacityNr++);

            setInt(("material." + name + number).c_str(), i);

        }
        if (diffuseNr == 1) setBool("material.hasDiffuse", false);
        else setBool("material.hasDiffuse", true);
        if (baseNr == 1) setBool("material.hasBaseColor", false);
        else setBool("material.hasBaseColor", true);
        if (specularNr == 1) setBool("material.hasSpecular", false);
        else setBool("material.hasSpecular", true);
        if (metallicNr == 1) setBool("material.hasMetallic", false);
        else setBool("material.hasMetallic", true);
        if(normalNr == 1) setBool("material.hasNormal", false);
        else setBool("material.hasNormal", true);
        if (depthNr == 1) setBool("material.hasDepth", false);
        else setBool("material.hasDepth", true);
        if (roughnessNr == 1) setBool("material.hasRoughness", false);
        else setBool("material.hasRoughness", true);
        if (aoNr == 1) setBool("material.hasAO", false);
        else setBool("material.hasAO", true);
        if (opacityNr == 1) setBool("material.hasOpacity", false);
        else setBool("material.hasOpacity", true);

        glActiveTexture(GL_TEXTURE0);

    }

    /*void setSSAOTexture(unsigned int ssaoTex)
    {
        glActiveTexture(GL_TEXTURE0 + ssaoTextureUnit);
        glBindTexture(GL_TEXTURE_2D, ssaoTex);
        
        setInt("ssaoTexture", ssaoTextureUnit);
        setBool("haveSSAO", true);
        
        glActiveTexture(GL_TEXTURE0);
    }*/

    void setTransform(vec3 position = vec3(0.f), vec3 rotation = vec3(0.f), vec3 scale = vec3(1.f))
    {
        mat4 model = mat4(1.f);
        
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), vec3(1.f, 0.f, 0.f));
        model = glm::rotate(model, glm::radians(rotation.y), vec3(0.f, 1.f, 0.f));
        model = glm::rotate(model, glm::radians(rotation.z), vec3(0.f, 0.f, 1.f));
        model = glm::scale(model, scale);
        

        setMat4("model", glm::value_ptr(model));
    }

    void setTransform(Transformation t)
    {
        glm::mat4 model = glm::mat4(1.f);

        model = glm::translate(model, t.translation);
        model = glm::rotate(model, glm::radians(t.rotation.x), vec3(1.f, 0.f, 0.f));
        model = glm::rotate(model, glm::radians(t.rotation.y), vec3(0.f, 1.f, 0.f));
        model = glm::rotate(model, glm::radians(t.rotation.z), vec3(0.f, 0.f, 1.f));
        model = glm::scale(model, t.scale);


        setMat4("model", glm::value_ptr(model));
    }

    // Utility uniform functions
    void setBool(const string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec3(const string& name, vec3 vector3) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), vector3.x, vector3.y, vector3.z);
    }
    // ------------------------------------------------------------------------
    void setMat4(const string& name, float* colMajorMatrix, int size = 1) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), size, GL_FALSE, colMajorMatrix);
    }

};
#endif