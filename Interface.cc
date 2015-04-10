#include <raindance/Raindance.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Interface/Document.hh>
#include <raindance/Core/FS.hh>

class TestDocument : public Document
{
public:
    TestDocument(const glm::vec3& dimension)
    : Document(dimension)
    {
        this->margin().top(5);
        this->margin().left(5);
        this->margin().right(5);
        this->margin().bottom(0);

        this->border().top(5);
        this->border().bottom(1);
        this->border().left(1);
        this->border().right(1);

        this->padding().top(0);
        this->padding().bottom(0);
        this->padding().left(0);
        this->padding().right(0);

        this->border().color(glm::vec4(WHITE, 1.0));

        this->content().color(glm::vec4(0.3, 0.3, 0.3, 1.0));
    }

    virtual ~TestDocument()
    {
    }

    void draw() override
    {
        auto position = this->position() + glm::vec3(this->margin().left() + this->border().left() + this->padding().left(),
                                                     this->margin().bottom() + this->border().bottom() + this->padding().bottom(),
                                                     0);
        glEnable(GL_SCISSOR_TEST);
        glViewport(position.x, position.y, this->content().getWidth(), this->content().getHeight());
        glScissor(position.x, position.y, this->content().getWidth(), this->content().getHeight());

        glClearColor(1.0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
    }

private:
};

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

        m_Camera.setOrthographicProjection(0, width, 0, height, -100, 100);
        m_Camera.lookAt(glm::vec3(0.0, 0.0, 100.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        FS::TextFile vert("Assets/interface_document.vert");
        FS::TextFile geom("Assets/interface_document.geom");
        FS::TextFile frag("Assets/interface_document.frag");
        m_DocumentShader = ResourceManager::getInstance().loadShader("Interface/document", vert.content(), frag.content(), geom.content());
        m_DocumentShader->dump();

        for (int i = 0; i < 80; i++)
        {
            auto width = RANDOM_FLOAT(50, 100);
            auto height = RANDOM_FLOAT(50, 100);

            auto doc = new TestDocument(glm::vec3(width, height, 0));

            body().elements().push_back(doc);
        }

        body().arrange(glm::vec3(0.0, 0.0, 0.0), glm::vec3(width, height, 0.0));

        update();
    }

    void update()
    {
        // --- Document Tree Visitor ---
        
        if (!body().refresh())
            return;

        m_DocumentBuffer.clear();

        for (auto doc : body().elements())
        {
            GPUDocument data;

            // Content + Padding + Border + Margin (Full)
            data.Position = doc->position();
            data.Dimension = doc->getDimension();
            data.Color = doc->margin().Color;
            // NOTE : We don't draw full document

            // Content + Padding + Border
            data.Position += glm::vec3(doc->margin().left(), doc->margin().bottom(), doc->margin().near());
            data.Dimension -= glm::vec3(doc->margin().getWidth(), doc->margin().getHeight(), doc->margin().getDepth());
            data.Color = doc->border().Color;
            // m_DocumentBuffer.push(&data, sizeof(GPUDocument)); // TODO: Draw Border
            
            // Content + Padding
            data.Position += glm::vec3(doc->border().left(), doc->border().bottom(), doc->border().near());
            data.Dimension -= glm::vec3(doc->border().getWidth(), doc->border().getHeight(), doc->border().getDepth());
            data.Color = doc->padding().Color;
            // NOTE: We don't draw padding

            // Content
            data.Position += glm::vec3(doc->padding().left(), doc->padding().bottom(), doc->padding().near());
            data.Dimension -= glm::vec3(doc->padding().getWidth(), doc->padding().getHeight(), doc->padding().getDepth());
            data.Color = doc->content().Color;
            m_DocumentBuffer.push(&data, sizeof(GPUDocument));
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

        clear();

        glDisable(GL_DEPTH_TEST);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_DocumentShader->use();

        m_DocumentShader->uniform("u_ModelViewMatrix").set(m_Camera.getViewMatrix() * transformation.state());
        m_DocumentShader->uniform("u_ProjectionMatrix").set(m_Camera.getProjectionMatrix());

        context->geometry().bind(m_DocumentBuffer, *m_DocumentShader);
        context->geometry().drawArrays(GL_POINTS, 0, m_DocumentBuffer.size() / sizeof(GPUDocument));        
        context->geometry().unbind(m_DocumentBuffer);

        body().draw();
    }

    virtual void idle(Context* context)
    {
        update();
    }

private:
    Camera m_Camera;

    Buffer m_DocumentBuffer;
    Shader::Program* m_DocumentShader;
};

int main(int argc, char** argv)
{
    auto demo = new Raindance(argc, argv);

    demo->add(new DemoWindow("Window", 1024, 728));

    demo->run();

    delete demo;
}
