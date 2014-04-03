#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Primitives/Grid.hh>
#include <raindance/Core/Primitives/Cube.hh>

const std::string g_VertexShader =
"attribute vec3 a_Position;                                             \n"
"                                                                       \n"
"uniform mat4 u_ModelViewProjectionMatrix;                              \n"
"uniform vec4 u_Color;                                                  \n"
"                                                                       \n"
"varying vec4 v_Color;                                                  \n"
"                                                                       \n"
"void main(void)                                                        \n"
"{                                                                      \n"
"    v_Color = u_Color;                                                 \n"
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


class Demo : public RainDance
{
public:
    struct Token
    {
        Token(float size, float height, const glm::vec4 color)
        : Size(size), Height(height), Color(color)
        {}

        float Size;
        float Height;
        glm::vec4 Color;
    };

    struct Pile
    {
        glm::vec3 Position;
        std::vector<Token> Tokens;
    };

    Demo()
    {
        m_Grid = NULL;
        m_Token = NULL;
        m_TokenShader = NULL;
    }

    virtual ~Demo()
    {
    }

    virtual void initialize()
    {
        m_Camera2D.setOrthographicProjection(0, 1024, 0, 728, 0.1f, 1024.0f);
        m_Camera2D.lookAt(glm::vec3(0.0, 0.0, 5.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Camera3D.setPerspectiveProjection(60.0f, (float)m_Window->width() / (float)m_Window->height(), 0.1f, 1024.0f);
        m_Camera3D.lookAt(glm::vec3(-50.0, 30.0, -50.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        m_Grid = new Grid(100, 100);
        m_Grid->setColor(glm::vec4(0.5, 0.5, 0.5, 1.0));

        float radius = 20;

        for (float alpha = 0; alpha < 2 * M_PI; alpha += 0.1)
        {
            auto pile = new Pile();
            float r = RANDOM_FLOAT(0.2, 1.5);
            pile->Position.x = r * radius * cos(alpha);
            pile->Position.y = 0;
            pile->Position.z = r * radius * sin(alpha);

            for (unsigned int i = 0; i < 5; i++)
            {
                float size = RANDOM_FLOAT(0.5, 1.5);
                float height = RANDOM_FLOAT(0.1, 2.0);
                glm::vec4 color = glm::vec4(RANDOM_FLOAT(0, 1), RANDOM_FLOAT(0, 1), RANDOM_FLOAT(0, 1), 1.0);
                pile->Tokens.push_back(Token(size, height, color));
            }

            m_Piles.push_back(pile);
        }

        m_Token = new Cube(glm::vec3(0, 0.5, 0));
        m_Token->getVertexBuffer().mute("a_Normal", true);
        m_TokenShader = ResourceManager::getInstance().loadShader("Board/Token", g_VertexShader, g_FragmentShader);
        m_TokenShader->dump();

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);
    }

    virtual void destroy()
    {
        SAFE_DELETE(m_Grid);
        ResourceManager::getInstance().unload(m_TokenShader);
    }

    virtual void draw()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Transformation transformation;

        transformation.push();
        transformation.rotate(90, glm::vec3(1.0, 0, 0));
        transformation.translate(glm::vec3(-50, -50, 0));
        m_Grid->draw(m_Context, transformation.state(), m_Camera3D.getViewMatrix(), m_Camera3D.getProjectionMatrix());
        transformation.pop();

        for (auto pile : m_Piles)
        {
            transformation.push();
            transformation.translate(pile->Position);
            for (auto& token : pile->Tokens)
            {
                transformation.push();
                transformation.scale(glm::vec3(token.Size, token.Height, token.Size));
                m_TokenShader->use();
                m_TokenShader->uniform("u_ModelViewProjectionMatrix").set(m_Camera3D.getViewProjectionMatrix() * transformation.state());
                m_TokenShader->uniform("u_Color").set(token.Color);

                m_Context.geometry().bind(m_Token->getVertexBuffer(), *m_TokenShader);
                m_Context.geometry().drawElements(GL_TRIANGLES, m_Token->getIndexBuffer().size() / sizeof(unsigned short int), GL_UNSIGNED_SHORT, m_Token->getIndexBuffer().ptr());
                m_Context.geometry().unbind(m_Token->getVertexBuffer());
                transformation.pop();

                transformation.translate(glm::vec3(0, token.Height, 0));
            }
            transformation.pop();
        }

        finish();
    }

    virtual void idle()
    {
        float t = 0.1f * static_cast<float>(m_Clock.milliseconds()) / 1000.0f;
        m_Camera3D.lookAt(glm::vec3(50 * cos(t), 10, 50 * sin(t)), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        glutPostRedisplay();
    }

private:
    Clock m_Clock;

    Camera m_Camera2D;
    Camera m_Camera3D;

    Grid* m_Grid;
    std::vector<Pile*> m_Piles;

    Cube* m_Token;
    Shader::Program* m_TokenShader;
};

int main(int argc, char** argv)
{
    Demo demo;
    demo.create(argc, argv);
    demo.addWindow("Board", 1024, 728);
    demo.initialize();
    demo.run();
    demo.destroy();
}
