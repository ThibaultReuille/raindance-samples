#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Primitives/Cube.hh>
#include <raindance/Core/FS.hh>
#include <raindance/Core/Clock.hh>

class DemoWindow : public Window
{
public:
    DemoWindow(const char* title, int width, int height, bool fullscreen = false)
    : Window(title, width, height, fullscreen)
    {
        m_Cube = NULL;
        m_Shader = NULL;
    }

    virtual ~DemoWindow()
    {
        ResourceManager::getInstance().unload(m_Shader);
        delete m_Cube;
    }

    void initialize(Context* context) override
    {
        auto viewport = this->getViewport();
        m_Camera.setPerspectiveProjection(60.0f, viewport.getDimension()[0] / viewport.getDimension()[1], 0.1f, 1024.0f);
        m_Camera.lookAt(glm::vec3(2, 2, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Cube = new Cube();
        {
            FS::TextFile vert("Assets/stereo_cube.vert");
            FS::TextFile frag("Assets/stereo_cube.frag");
            m_Shader = ResourceManager::getInstance().loadShader("Stereo/cube", vert.content(), frag.content());
            m_Shader->dump();
        }

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    void drawCube(Context* context, Camera& camera, Transformation& transformation, GLenum buffer, const glm::vec4 tint)
    {
        glDrawBuffer(buffer);
        m_Shader->use();
        m_Shader->uniform("u_Tint").set(tint);
        m_Cube->draw(context, m_Camera, transformation, m_Shader, Cube::TRIANGLES);
    }

    void draw(Context* context) override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     
        Transformation transformation;

        glm::vec3 eye(0, 1.0, 3);
        
        float eye_half_distance = 0.5;
        glm::vec3 right(1.0, 0.0, 0.0);
        glm::vec3 left(-1.0, 0.0, 0.0);

        m_Camera.lookAt(eye + eye_half_distance * left, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        drawCube(context, m_Camera, transformation, GL_BACK_LEFT, glm::vec4(RED, 1.0));

        m_Camera.lookAt(eye + eye_half_distance * right, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        drawCube(context, m_Camera, transformation, GL_BACK_RIGHT, glm::vec4(GREEN, 1.0));

        checkGLErrors();
    }

    void idle(Context* context) override
    {
        
    }

private:
    Camera m_Camera;
    Cube* m_Cube;
    Shader::Program* m_Shader;
    Clock m_Clock;
};

int main(int argc, char** argv)
{
    auto demo = new Raindance(argc, argv);

    demo->add(new DemoWindow("Cube", 1024, 728));
    demo->run();
    
    delete demo;

    return 0;
}
