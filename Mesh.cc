#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Primitives/Cube.hh>
#include <raindance/Core/Transformation.hh>

const std::string g_VertexShader =                          
"uniform mat4 u_ModelViewProjectionMatrix;                                              \n"
"                                                                                       \n"
"attribute vec3 a_Position;                                                             \n"
"attribute vec3 a_Translation;                                                          \n"
"attribute vec4 a_Color;                                                                \n"
"                                                                                       \n"
"varying vec4 v_Color;                                                                  \n"
"                                                                                       \n"
"void main(void)                                                                        \n"
"{                                                                                      \n"
"    v_Color = a_Color;                                                                 \n"
"    gl_Position = u_ModelViewProjectionMatrix * vec4(a_Position + a_Translation, 1.0); \n"
"}                                                                                      \n";

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

class Mesh
{
public:

    struct Vertex
    {
        glm::vec3 Position;
        // glm::vec3 Normal;
    };

    struct Instance
    {
        glm::vec3 Translation;
        glm::vec4 Color;
    };

    Mesh()
    {
        m_Shader = ResourceManager::getInstance().loadShader("cube", g_VertexShader, g_FragmentShader);
        // m_Shader->dump();

        {
            m_VertexBuffer << glm::vec3(-0.5, -0.5, 0.0);
            m_VertexBuffer << glm::vec3( 0.5, -0.5, 0.0);
            m_VertexBuffer << glm::vec3(-0.5,  0.5, 0.0);
            m_VertexBuffer << glm::vec3( 0.5,  0.5, 0.0);
        
            m_VertexBuffer.describe("a_Position", 3, GL_FLOAT, sizeof(Vertex), 0);
     
            m_VertexBuffer.generate(Buffer::STATIC);
        }
        {
            for (int n = 0; n < 10000; n++)
            {
                float d = 50;
                glm::vec3 p;
                p.x = d * RANDOM_FLOAT(-1.0, 1.0);
                p.y = d * RANDOM_FLOAT(-1.0, 1.0);
                p.z = d * RANDOM_FLOAT(-1.0, 1.0);

                glm::vec4 c;
                c.r = RANDOM_FLOAT(0.0, 1.0);
                c.g = RANDOM_FLOAT(0.0, 1.0);
                c.b = RANDOM_FLOAT(0.0, 1.0);
                c.a = 1.0;

                m_InstanceBuffer << p << c;
            }

            m_InstanceBuffer.describe("a_Translation", 3, GL_FLOAT, sizeof(Instance), 0);
            m_InstanceBuffer.describe("a_Color",       4, GL_FLOAT, sizeof(Instance), sizeof(glm::vec3));

            m_InstanceBuffer.generate(Buffer::STATIC);
        }
    }

    virtual ~Mesh()
    {
        ResourceManager::getInstance().unload(m_Shader);
    }

    virtual void draw(Context* context, Camera& camera, Transformation& transformation)
    {
        m_Shader->use();
        m_Shader->uniform("u_ModelViewProjectionMatrix").set(camera.getViewProjectionMatrix() * transformation.state());

        context->geometry().bind(m_VertexBuffer, *m_Shader);        
        context->geometry().bind(m_InstanceBuffer, *m_Shader);
        
        glVertexAttribDivisorARB(m_Shader->attribute("a_Position").location(), 0); // Same vertices per instance
        // glVertexAttribDivisorARB(m_VertexBuffer.descriptions()[1].Location, 0);

        glVertexAttribDivisorARB(m_Shader->attribute("a_Translation").location(), 1);
        glVertexAttribDivisorARB(m_Shader->attribute("a_Color").location(), 0);

        context->geometry().drawArraysInstanced(GL_TRIANGLE_STRIP, 0, m_VertexBuffer.size() / sizeof(Vertex), m_InstanceBuffer.size() / sizeof(Instance));
        
        context->geometry().unbind(m_VertexBuffer);
        context->geometry().unbind(m_InstanceBuffer);
    }

private:
    Shader::Program* m_Shader;
    Buffer m_VertexBuffer;
    Buffer m_InstanceBuffer;
};

class DemoWindow : public Window
{
public:
    DemoWindow(const char* title, int width, int height)
    : Window(title, width, height)
    {
        m_Camera.setPerspectiveProjection(60.0f, (float)this->width() / this->height(), 0.1f, 1024.0f);
        m_Camera.lookAt(glm::vec3(1.5, 2, 2.5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Mesh = new Mesh();

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    virtual ~DemoWindow()
    {
        delete m_Mesh;
    }

    virtual void draw(Context* context)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Transformation transformation;

        m_Mesh->draw(context, m_Camera, transformation);
    }

    virtual void idle(Context* context)
    {
        float t = context->clock().seconds() / 4;
        float r = 100;

        glm::vec3 pos = glm::vec3(r * cos(t), 0, r * sin(t));

        m_Camera.lookAt(pos, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    }

private:
    Camera m_Camera;
    Mesh* m_Mesh;
};

class Demo : public RainDance
{
public:
    virtual void initialize()
    {
        auto id = m_WindowManager.add(new DemoWindow("Cube", 1024, 728));
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
        static_cast<DemoWindow*>(m_WindowManager.active())->idle(&m_Context);
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
