#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Text.hh>
#include <raindance/Core/Font.hh>

class DemoWindow : public rd::Window
{
public:
    DemoWindow(const char* title, int width, int height, bool fullscreen = false)
    : Window(title, width, height, fullscreen), m_Font(NULL)
    {
        m_Camera.setOrthographicProjection(0, 1000, 0, 1000, -100, 600);
        m_Camera.lookAt(glm::vec3(0.0, 0.0, 200.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);

        m_Font = new rd::Font();
        m_Text.set("Data is Beautiful.", m_Font);
    }

    virtual ~DemoWindow()
    {
        SAFE_DELETE(m_Font);
    }

    virtual void initialize(Context* context)
    {
        (void) context;
    }

    virtual void draw(Context* context)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        Transformation transformation;
        glm::vec4 color;
        float t;
        for (int i = 0; i < 13; i++)
        {   
            t = static_cast<float>(i) / 13.0;

            color.r = t;
            color.g = t * t;
            color.b = 1.0 - t;
            color.a = 1.0;

            m_Text.setColor(color);
            m_Text.draw(*context, m_Camera.getProjectionMatrix() * m_Camera.getViewMatrix() * transformation.state());
            transformation.translate(glm::vec3(0, m_Font->getSize(), 0.0));
            transformation.scale(glm::vec3(1.05, 1.05, 1.0));
        }
    }

    virtual void idle(Context* context)
    {
        (void) context;
    }

private:
    Camera m_Camera;

    Text m_Text;
    rd::Font* m_Font;
};

int main(int argc, char** argv)
{
    auto demo = new Raindance(argc, argv);
    demo->add(new DemoWindow("Fonts", 1024, 728));
    demo->run();
    delete demo;
}
