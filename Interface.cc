#include <raindance/Raindance.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Interface/Box.hh>
#include <raindance/Core/FS.hh>

class DemoWindow : public rd::Window
{
public:

    struct GPUDocument
    {
        glm::vec3 Position;
        glm::vec3 Dimension;
        glm::vec4 Color;
    };

    DemoWindow(const char* title, int width, int height, bool fullscreen = false)
    : Window(title, width, height, fullscreen)
    {
    }

    virtual ~DemoWindow()
    {
    }
    
    virtual void initialize(Context* context)
    {
        glClearColor(0.1, 0.1, 0.1, 1.0);

        auto width = this->getViewport().getDimension()[0];
        auto height = this->getViewport().getDimension()[1];


        m_Camera.setOrthographicProjection(0, width, height, 0, -100, 100);
        m_Camera.lookAt(glm::vec3(0.0, 0.0, 100.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        FS::TextFile vert("Assets/interface_document.vert");
        FS::TextFile geom("Assets/interface_document.geom");
        FS::TextFile frag("Assets/interface_document.frag");
        m_DocumentShader = ResourceManager::getInstance().loadShader("Interface/document", vert.content(), frag.content(), geom.content());
        m_DocumentShader->dump();

        for (int i = 0; i < 80; i++)
        {
            auto width = RANDOM_INT(50, 100);
            auto height = RANDOM_INT(50, 100);

            auto doc = new rd::Document(glm::vec3(width, height, 0));
            {
                doc->margin().top(5);
                doc->margin().left(5);
                doc->margin().right(5);
                doc->margin().bottom(0);

                doc->border().top(1);
                doc->border().bottom(1);
                doc->border().left(1);
                doc->border().right(1);

                doc->padding().top(0);
                doc->padding().bottom(0);
                doc->padding().left(0);
                doc->padding().right(0);

                doc->content().color(glm::vec4(PINK, 1.0));
            }
            m_Documents.push_back(doc);
        }

        update();
    }

    void update()
    {
        // --- Document Tree Visitor ---
        
        m_DocumentBuffer.clear();

        auto cursor = glm::vec3(0, 0, 0);
        auto max_width = 0;

        for (auto doc : m_Documents)
        {
            auto dimension = doc->getDimension();

            if (dimension.x > max_width)
                max_width = dimension.x;

            if (cursor.y + dimension.y > this->getViewport().getDimension()[1])
            {
                cursor.x += max_width;
                cursor.y = 0;
                max_width = 0;
            }

            doc->position(cursor);

            GPUDocument data;

            // Content + Padding + Border + Margin (Full)
            data.Position = cursor;
            data.Dimension = dimension;
            data.Color = doc->margin().Color;
            // NOTE : We don't draw full document

            // Content + Padding + Border
            data.Position += glm::vec3(doc->margin().X[0], doc->margin().Y[0], doc->margin().Z[0]);
            data.Dimension -= glm::vec3(doc->margin().getWidth(), doc->margin().getHeight(), doc->margin().getDepth());
            data.Color = doc->border().Color;
            // m_DocumentBuffer.push(&data, sizeof(GPUDocument)); // TODO: Draw border
            
            // Content + Padding
            data.Position += glm::vec3(doc->border().X[0], doc->border().Y[0], doc->border().Z[0]);
            data.Dimension -= glm::vec3(doc->border().getWidth(), doc->border().getHeight(), doc->border().getDepth());
            data.Color = doc->padding().Color;
            // NOTE: We don't draw padding

            // Content
            data.Position += glm::vec3(doc->padding().X[0], doc->padding().Y[0], doc->padding().Z[0]);
            data.Dimension -= glm::vec3(doc->padding().getWidth(), doc->padding().getHeight(), doc->padding().getDepth());
            data.Color = doc->content().Color;
            m_DocumentBuffer.push(&data, sizeof(GPUDocument));

            // ---

            cursor.y += dimension.y;
        }

        // --- Document Buffer Update ---

        static bool m_DocumentBufferFirstUpdate = true;

        if (!m_DocumentBufferFirstUpdate)
            m_DocumentBuffer.update();

        m_DocumentBuffer.describe("a_Position",  3, GL_FLOAT, sizeof(GPUDocument), 0);
        m_DocumentBuffer.describe("a_Dimension", 3, GL_FLOAT, sizeof(GPUDocument), sizeof(glm::vec3));
        m_DocumentBuffer.describe("a_Color",     4, GL_FLOAT, sizeof(GPUDocument), sizeof(glm::vec3) * 2);
        
        if (m_DocumentBufferFirstUpdate)
            m_DocumentBuffer.generate(Buffer::DYNAMIC);
        
        m_DocumentBufferFirstUpdate = false;   
    }

    virtual void draw(Context* context)
    {
        Transformation transformation;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_DocumentShader->use();

        m_DocumentShader->uniform("u_ModelViewMatrix").set(m_Camera.getViewMatrix() * transformation.state());
        m_DocumentShader->uniform("u_ProjectionMatrix").set(m_Camera.getProjectionMatrix());

        context->geometry().bind(m_DocumentBuffer, *m_DocumentShader);
        context->geometry().drawArrays(GL_POINTS, 0, m_DocumentBuffer.size() / sizeof(GPUDocument));        
        context->geometry().unbind(m_DocumentBuffer);
    }

    virtual void idle(Context* context)
    {
        if (m_NeedsUpdate)
        {
            update();
            m_NeedsUpdate = false;
        }
    }

    virtual void onCursorPos(double xpos, double ypos)
    { 
        auto width = this->getViewport().getDimension()[0];
        auto height = this->getViewport().getDimension()[1];

        m_NeedsUpdate = false;

        for (auto doc : m_Documents)
        {
            glm::vec2 pos = glm::vec2((float)xpos, (float)ypos) - doc->position().xy();

            switch(doc->pick(pos))
            {
            case rd::Document::OUTSIDE:
                doc->content().Color = glm::vec4(PINK, 1.0);
                break;
            default:
                doc->content().Color = glm::vec4(WHITE, 1.0);
                break;
            }
        }

        m_NeedsUpdate = true;
    }

private:
    Camera m_Camera;

    Buffer m_DocumentBuffer;
    Shader::Program* m_DocumentShader;
    std::vector<rd::Document*> m_Documents;

    bool m_NeedsUpdate;
};

int main(int argc, char** argv)
{
    auto demo = new Raindance(argc, argv);

    demo->add(new DemoWindow("Window", 1024, 728));

    demo->run();

    delete demo;
}
