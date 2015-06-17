#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Primitives/Axis.hh>
#include <raindance/Core/Primitives/Grid.hh>
#include <raindance/Core/Primitives/Cube.hh>
#include <raindance/Core/FS.hh>
#include <raindance/Core/Clock.hh>
#include <raindance/Core/VR/OculusRift.hh>

class DemoWindow : public rd::Window
{
public:
    DemoWindow(rd::Window::Settings* settings)
    : Window(settings)
    {
        m_Axis = NULL;
        m_Grid = NULL;
        m_Cube = NULL;
        m_Shader = NULL;
    }

    virtual ~DemoWindow()
    {
        ResourceManager::getInstance().unload(m_Shader);
        delete m_Cube;
        delete m_Grid;
        delete m_Axis;
    }

    void initialize(Context* context) override
    {
        auto viewport = this->getViewport();
        m_Camera.setPerspectiveProjection(60.0f, (0.5f * viewport.getDimension()[0]) / viewport.getDimension()[1], 0.1f, 1024.0f);
        m_Camera.lookAt(glm::vec3(2, 2, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Axis = new Axis();

        Grid::Parameters parameters;
        parameters.Dimension = glm::vec2(100, 100);
        parameters.Scale = glm::vec2(1, 1);
        parameters.Step = glm::vec2(10, 10);
        parameters.Division = glm::vec2(2, 2);
        parameters.Shift = glm::vec2(0, 0);
        parameters.Color = glm::vec4(BLACK, 1.0);
        parameters.BackgroundColor = glm::vec4(0.1, 0.1, 0.1, 0.5);
        m_Grid = new Grid(parameters);

        m_Cube = new Cube();
        {
            FS::TextFile vert("Assets/stereo_cube.vert");
            FS::TextFile frag("Assets/stereo_cube.frag");
            m_Shader = ResourceManager::getInstance().loadShader("Stereo/cube", vert.content(), frag.content());
            m_Shader->dump();
        }

        m_UserPosition = glm::vec3(0.0, 2.0, 5.0);
        m_UserTarget = glm::vec3(0.0, 2.0, 0.0);
        m_UserUp = glm::vec3(0.0, 1.0, 0.0);

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    void drawScene(Context* context, Camera& camera, Transformation& transformation)
    {
        //glDrawBuffer(buffer);

        m_Axis->draw(context, camera, transformation);

        transformation.push();
        transformation.translate(glm::vec3(-50.0f, 0.0, -50.0));
        transformation.rotate(90, glm::vec3(1, 0, 0));
        m_Grid->draw(context, camera, transformation);
        transformation.pop();

        transformation.push();
                
        m_Shader->use();
        m_Shader->uniform("u_Tint").set(glm::vec4(WHITE, 1.0));

        for (float x = -50; x <= 50; x += 5.0)
            for (float z = -50; z <= 50; z += 5.0)
            {
                transformation.push();

                transformation.translate(glm::vec3(x, 0.51, z));
                m_Cube->draw(context, camera, transformation, m_Shader, Cube::TRIANGLES);
        
                transformation.pop();
            }

        transformation.pop();   
    }

    void draw(Context* context) override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     
        Transformation transformation;

        float t = m_Clock.seconds();

        auto framebuffer = this->getViewport().getFramebuffer();

        glViewport(0, 0, framebuffer.Width / 2, framebuffer.Height);
        {
            m_Rift->lookAt(OculusRift::LEFT, m_UserPosition, m_UserTarget, m_UserUp, m_Camera);
            drawScene(context, m_Camera, transformation);
        }

        glViewport(framebuffer.Width / 2, 0, framebuffer.Width / 2, framebuffer.Height);
        {
            m_Rift->lookAt(OculusRift::RIGHT, m_UserPosition, m_UserTarget, m_UserUp, m_Camera);
            drawScene(context, m_Camera, transformation);
        }

        checkGLErrors();
    }

    void idle(Context* context) override
    {
        m_Rift->idle();
    }

    void onKey(int key, int scancode, int action, int mods) override
    {
        bool isPressOrRepeat = action == GLFW_PRESS || action == GLFW_REPEAT;

        if (isPressOrRepeat && key == GLFW_KEY_UP)
        {
            m_UserPosition += m_Camera.front();
            m_UserTarget += m_Camera.front();
        }
        else if (isPressOrRepeat && key == GLFW_KEY_DOWN)
        {
            m_UserPosition += m_Camera.back();
            m_UserTarget += m_Camera.back();
        }   
        else if (isPressOrRepeat && key == GLFW_KEY_LEFT)
        {
            m_UserPosition += m_Camera.left();
            m_UserTarget += m_Camera.left();
        }
        else if (isPressOrRepeat && key == GLFW_KEY_RIGHT)
        {
            m_UserPosition += m_Camera.right();
            m_UserTarget += m_Camera.right();
        }
    }

    void setOculusRift(OculusRift* rift) { m_Rift = rift; }

private:
    Camera m_Camera;
    Axis* m_Axis;
    Grid* m_Grid;
    Cube* m_Cube;
    Shader::Program* m_Shader;
    Clock m_Clock;

    OculusRift* m_Rift;

    glm::vec3 m_UserPosition;
    glm::vec3 m_UserTarget;
    glm::vec3 m_UserUp;
};

int main(int argc, char** argv)
{
    OculusRift rift;

    auto demo = new Raindance(argc, argv);

    rd::Window::Settings settings;
    settings.Title = std::string("Stereo");
    settings.Width = 0;
    settings.Height = 0;
    settings.Fullscreen = true;
    settings.Monitor = rift.findMonitor();

    auto window = new DemoWindow(&settings);

    window->setOculusRift(&rift);

    demo->add(window);
    demo->run();

    delete demo;

    return 0;
}
