#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Primitives/Cube.hh>
#include <raindance/Core/FS.hh>

class DemoWindow : public rd::Window
{
public:
    DemoWindow(const char* title, int width, int height, bool fullscreen = false)
    : Window(title, width, height, fullscreen)
    {
        m_Cube = NULL;
        m_Shader1 = NULL;
        m_Shader2 = NULL;
    }

    virtual ~DemoWindow()
    {
        ResourceManager::getInstance().unload(m_Shader1);
        ResourceManager::getInstance().unload(m_Shader2);
        delete m_Cube;
    }

    void initialize(Context* context) override
    {
        (void) context;
        
        auto viewport = this->getViewport();
        m_Camera.setPerspectiveProjection(60.0f, viewport.getDimension()[0] / viewport.getDimension()[1], 0.1f, 1024.0f);
        m_Camera.lookAt(glm::vec3(1, 2, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Cube = new Cube();
        m_Cube->getLineVertexBuffer().mute("a_Normal", true);

        {
            FS::TextFile vert("Assets/cube_solid.vert");
            FS::TextFile frag("Assets/cube_generic.frag");
            m_Shader1 = ResourceManager::getInstance().loadShader("Cube/cube_solid", vert.content(), frag.content());
            m_Shader1->dump();
        }
        {
            FS::TextFile vert("Assets/cube_wireframe.vert");
            FS::TextFile frag("Assets/cube_generic.frag");
            m_Shader2 = ResourceManager::getInstance().loadShader("Cube/cube_wireframe", vert.content(), frag.content());
            m_Shader2->dump();

            m_Shader2->use();
            m_Shader2->uniform("u_Color").set(glm::vec4(1.0, 1.0, 1.0, 1.0));
        }

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    void draw(Context* context) override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     
        Transformation transformation;

        transformation.translate(glm::vec3(-1, 0, 0));
        m_Cube->draw(context, m_Camera, transformation, m_Shader1, Cube::TRIANGLES);
        
        transformation.translate(glm::vec3(+2, 0, 0));
        m_Cube->draw(context, m_Camera, transformation, m_Shader2, Cube::LINES);

        checkGLErrors();
    }

    void idle(Context* context) override
    {
        (void) context;
    }

private:
    Camera m_Camera;
    Cube* m_Cube;
    Shader::Program* m_Shader1;
    Shader::Program* m_Shader2;
};

int main(int argc, char** argv)
{
    auto demo = new Raindance(argc, argv);

    demo->add(new DemoWindow("Cube", 1024, 728));
    demo->run();
    
    delete demo;

    return 0;
}
