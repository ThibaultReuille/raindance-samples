#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Primitives/Cube.hh>

const std::string g_VertexShader =                          
"attribute vec3 a_Position;                                             \n"
"attribute vec3 a_Normal;                                               \n"
"                                                                       \n"
"uniform mat4 u_ModelViewProjectionMatrix;                              \n"
"                                                                       \n"
"varying vec4 v_Color;                                                  \n"
"                                                                       \n"
"void main(void)                                                        \n"
"{                                                                      \n"
"    v_Color = 0.5 + 0.5 * vec4(a_Normal, 1.0);                         \n"
"    gl_Position = u_ModelViewProjectionMatrix * vec4(a_Position, 1.0); \n"
"}                                                                      \n";

const std::string g_FragmentShader =
"#ifdef GL_ES                \n"
"precision mediump float;    \n"
"#endif                      \n"
"                            \n"
"varying vec4 v_Color;       \n"
"                            \n"
"void main(void)             \n"
"{                           \n"
"    gl_FragColor = v_Color; \n"
"}                           \n";

class DemoWindow : public Window
{
public:
    DemoWindow(const char* title, int width, int height)
    : Window(title, width, height)
    {
        m_Cube = NULL;
        m_Shader = NULL;

        m_Camera.setPerspectiveProjection(60.0f, (float)this->width() / this->height(), 0.1f, 1024.0f);
        m_Camera.lookAt(glm::vec3(1.5, 2, 2.5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Cube = new Cube();

        m_Shader = ResourceManager::getInstance().loadShader("cube", g_VertexShader, g_FragmentShader);
        // m_Shader->dump();

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    virtual ~DemoWindow()
    {
        ResourceManager::getInstance().unload(m_Shader);
        delete m_Cube;
    }

    virtual void draw(Context* context)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_Shader->use();
        m_Shader->uniform("u_ModelViewProjectionMatrix").set(m_Camera.getViewProjectionMatrix());

        context->geometry().bind(m_Cube->getVertexBuffer(), *m_Shader);
        context->geometry().drawElements(GL_TRIANGLES, m_Cube->getTriangleBuffer().size() / sizeof(unsigned short int), GL_UNSIGNED_SHORT, m_Cube->getTriangleBuffer().ptr());
        context->geometry().unbind(m_Cube->getVertexBuffer());
    }

private:
    Camera m_Camera;
    Cube* m_Cube;
    Shader::Program* m_Shader;
};

class Demo : public RainDance
{
public:
    virtual void initialize()
    {
        auto id = m_WindowManager.add(new DemoWindow("Cube", 800, 600));
        m_WindowManager.bind(id);
    }

    virtual void draw()
    {
        Geometry::beginFrame();

        m_WindowManager.active()->preDraw(&m_Context);
        m_WindowManager.active()->draw(&m_Context);
        m_WindowManager.active()->postDraw(&m_Context);

        finish();

        Geometry::endFrame();
    }

    virtual void idle()
    {
        postRedisplay();
    }
};

int main(int argc, char** argv)
{
    Demo demo;

    demo.create(argc, argv);

    demo.initialize();
    demo.run();

    demo.destroy();
}
