#include <raindance/Raindance.hh>
#include <raindance/Core/OpenCL.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Camera/Controllers.hh>
#include <raindance/Core/Primitives/Sphere.hh>

const std::string g_VertexShader =
    "attribute vec3 a_Position;                                                           \n"
    "attribute vec3 a_Normal;                                                             \n"
    "attribute vec2 a_Texcoord;                                                           \n"
    "                                                                                     \n"
    "uniform mat4 u_ModelViewProjectionMatrix;                                            \n"
    "                                                                                     \n"
    "varying vec4 v_Color;                                                                \n"
    "                                                                                     \n"
    "void main(void)                                                                      \n"
    "{                                                                                    \n"
    "    v_Color = 0.5 + 0.25 * vec4(a_Normal, 1.0) + 0.25 * vec4(a_Texcoord, 1.0, 1.0);  \n"
    "    gl_Position = u_ModelViewProjectionMatrix * vec4(a_Position, 1.0);               \n"
    "}                                                                                    \n";

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

const std::string g_ComputeProgram =
    "__kernel void repulsion(__global float4* inNodes,                                  \n"
    "                        __global float4* outDirections,                            \n"
    "                        const unsigned long count,                                 \n"
    "                        const float k)                                             \n"
    "{                                                                                  \n"
    "    unsigned int i = get_global_id(0);                                             \n"
    "    float4 f = (float4)(0.0);                                                      \n"
    "    for(unsigned long j = 0; j < count; j++)                                       \n"
    "    {                                                                              \n"
    "       if (i != j)                                                                 \n"
    "       {                                                                           \n"
    "           float4 direction = inNodes[i] - inNodes[j];                             \n"
    "           float magnitude = length(direction);                                    \n"
    "           if (magnitude > 0.0)                                                    \n"
    "               f += (direction / magnitude) * (k * k / magnitude);                 \n"
    "       }                                                                           \n"
    "    }                                                                              \n"
    "    outDirections[i] = f;                                                          \n"
    "}                                                                                  \n"

    "__kernel void attraction(__global float4* inNodes,                                 \n"
    "                         __global ulong2* inEdges,                                 \n"
    "                         __global float4* outDirections,                           \n"
    "                         const unsigned long count,                                \n"
    "                         const float k)                                            \n"
    "{                                                                                  \n"
    "    unsigned int i = get_global_id(0);                                             \n"
    "    unsigned long src = inEdges[i].x;                                              \n"
    "    unsigned long dst = inEdges[i].y;                                              \n"
    "    float4 pos1 = inNodes[src];                                                    \n"
    "    float4 pos2 = inNodes[dst];                                                    \n"
    "    float4 direction = pos1 - pos2;                                                \n"
    "    float magnitude = length(direction);                                           \n"
    "    if (magnitude > 100.0)                                                         \n"
    "    {                                                                              \n"
    "        outDirections[src] -= direction * magnitude / k;                           \n"
    "        outDirections[dst] += direction * magnitude / k;                           \n"
    "    }                                                                              \n"
    "}                                                                                  \n"

    "__kernel void movement(__global float4* inNodes,                                   \n"
    "                       __global float4* inForces,                                  \n"
    "                       __global float4* outNodes,                                  \n"
    "                       const float temperature)                                    \n"
    "{                                                                                  \n"
    "     unsigned int i = get_global_id(0);                                            \n"
    "     if (length(inNodes[i]) < 100000)                                              \n"
    "     {                                                                             \n"
    "         float magnitude = length(inForces[i]);                                    \n"
    "         if (magnitude > 0.0)                                                      \n"
    "           outNodes[i] = inNodes[i] + temperature * inForces[i] / magnitude;       \n"
    "     }                                                                             \n"
    "}                                                                                  \n";

class DemoWindow : public GLFW::Window
{
public:
    struct Node
    {
        glm::vec4 Position;
    };

    struct Edge
    {
        unsigned long Node1;
        unsigned long Node2;
    };

    struct Force
    {
        glm::vec4 Direction;
    };

    DemoWindow(const char* title, int width, int height)
    : GLFW::Window(title, width, height)
    {
    }
    
    virtual ~DemoWindow()
    {
        ResourceManager::getInstance().unload(m_Shader);
        delete m_Sphere;
    }

    virtual void initialize(Context* context)
    {
        m_NumParticles = 4096;
        m_NumForces = 4096;
        m_Time = 0.0;
        float volume = 10 * 10 * 10;
        m_K = pow(volume / m_NumParticles, 1.0 / 3.0);
        m_Iterations = 0;
        m_Temperature = 2.0;

        // Scene initialization
        {
            m_Camera.setPerspectiveProjection(60.0f, (float)width() / (float)height(), 0.1f, 1000000.0f);

            m_SphericalCameraController.bind(context, &m_Camera);
            m_SphericalCameraController.setRadius(2000);
            m_SphericalCameraController.updateCamera();

            m_Sphere = new SphereMesh(10.0, 4, 4);
            m_Shader = ResourceManager::getInstance().loadShader("sphere", g_VertexShader, g_FragmentShader);
            // m_Shader->dump();

            // Particle instancing
            m_Nodes.resize(m_NumParticles);
            unsigned long count = 0;
            for (auto& n : m_Nodes)
            {
                float x = 1000 * ((float) rand() / RAND_MAX - 0.5f);
                float y = 1000 * ((float) rand() / RAND_MAX - 0.5f);
                float z = 1000 * ((float) rand() / RAND_MAX - 0.5f);
                n.Position = glm::vec4(x, y, z, 0.0);
                count ++;
            }

            count = 0;
            m_Edges.resize(m_NumParticles);
            for (auto& e : m_Edges)
            {
                e.Node1 = count;
                e.Node2 = m_NumParticles % (count + 1);
                count ++;
            }
        }

        // OpenGL initialization
        {
            glClearColor(0.2, 0.2, 0.2, 1.0);
            glEnable(GL_DEPTH_TEST);
        }

        // OpenCL initialization
        {
            m_OpenCL.detect();
            m_OpenCL.dump();
            // TODO : Find best OpenCL architecture instead of hardcoded one
            // OpenCL::Device* device = m_OpenCL.getBestDevice();
            const OpenCL::Device* device = m_OpenCL.device(1);
            OpenCL::Context* context = m_OpenCL.createContext(*device);
            m_Queue = m_OpenCL.createCommandQueue(*context);

            OpenCL::Program* program = m_OpenCL.loadProgram(*context, "particles", g_ComputeProgram);
            m_RepulsionK = m_OpenCL.createKernel(*program, "repulsion");
            m_AttractionK = m_OpenCL.createKernel(*program, "attraction");
            m_MovementK = m_OpenCL.createKernel(*program, "movement");

            m_InputNodeBuffer = m_OpenCL.createBuffer(*context, CL_MEM_READ_ONLY, m_Nodes.size() * sizeof(Node));
            m_InputEdgeBuffer = m_OpenCL.createBuffer(*context, CL_MEM_READ_ONLY, m_Edges.size() * sizeof(Edge));
            m_ForceBuffer = m_OpenCL.createBuffer(*context, CL_MEM_READ_WRITE, m_Nodes.size() * sizeof(Node));
            m_OutputNodeBuffer = m_OpenCL.createBuffer(*context, CL_MEM_WRITE_ONLY, m_Nodes.size() * sizeof(Node));

            // Get the maximum work group size for executing the kernel on the device
            /*
            size_t local; // local domain size for our calculation
            error = clGetKernelWorkGroupInfo(m_OpenCL.kernel(m_KID).Object, m_OpenCL.device(did).ID, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
            if (error != CL_SUCCESS)
            {
                printf("[OpenCL] Error: Failed to retrieve kernel work group info! %d\n", error);
                throw;
            }
            LOG("LOCAL : %zu\n", local);
             */
        }
    }

    virtual void draw(Context* context)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_Shader->use();

        glm::mat4 model;

        for (auto n : m_Nodes)
        {
            model = glm::translate(glm::mat4(), glm::vec3(n.Position));
            m_Shader->uniform("u_ModelViewProjectionMatrix").set(m_Camera.getViewProjectionMatrix() * model);

            context->geometry().bind(m_Sphere->getVertexBuffer(), *m_Shader);
            context->geometry().drawElements(GL_TRIANGLES, m_Sphere->getIndexBuffer().size() / sizeof(unsigned short int), GL_UNSIGNED_SHORT, m_Sphere->getIndexBuffer().ptr());
            context->geometry().unbind(m_Sphere->getVertexBuffer());
        }
    }

    virtual void idle(Context* context)
    {
        (void) context;
        
        if (m_Iterations == 0)
        {
            m_RepulsionK->setArgument(0, *m_InputNodeBuffer);
            m_RepulsionK->setArgument(1, *m_ForceBuffer);
            m_RepulsionK->setArgument(2, &m_NumParticles, sizeof(unsigned long));
            m_RepulsionK->setArgument(3, &m_K, sizeof(float));

            m_AttractionK->setArgument(0, *m_InputNodeBuffer);
            m_AttractionK->setArgument(1, *m_InputEdgeBuffer);
            m_AttractionK->setArgument(2, *m_ForceBuffer);
            m_AttractionK->setArgument(3, &m_NumForces, sizeof(unsigned long));
            m_AttractionK->setArgument(4, &m_K, sizeof(float));

            m_MovementK->setArgument(0, *m_InputNodeBuffer);
            m_MovementK->setArgument(1, *m_ForceBuffer);
            m_MovementK->setArgument(2, *m_OutputNodeBuffer);
            m_MovementK->setArgument(3, &m_Temperature, sizeof(float));

            m_Clock.reset();
        }

        m_OpenCL.enqueueWriteBuffer(*m_Queue, *m_InputNodeBuffer, CL_TRUE, 0, m_Nodes.size() * sizeof(Node), m_Nodes.data(), 0, NULL, NULL);
        m_OpenCL.enqueueWriteBuffer(*m_Queue, *m_InputEdgeBuffer, CL_TRUE, 0, m_Edges.size() * sizeof(Edge), m_Edges.data(), 0, NULL, NULL);

        m_OpenCL.enqueueNDRangeKernel(*m_Queue, *m_RepulsionK, 1, NULL, &m_NumParticles, NULL, 0, NULL, NULL);
        m_OpenCL.enqueueNDRangeKernel(*m_Queue, *m_AttractionK, 1, NULL, &m_NumForces, NULL, 0, NULL, NULL);
        m_OpenCL.enqueueNDRangeKernel(*m_Queue, *m_MovementK, 1, NULL, &m_NumParticles, NULL, 0, NULL, NULL);

        clFinish(m_Queue->Object);
        m_OpenCL.enqueueReadBuffer(*m_Queue, *m_OutputNodeBuffer, CL_TRUE, 0, m_Nodes.size() * sizeof(Node), m_Nodes.data(), 0, NULL, NULL);

        m_Time = (float)m_Clock.milliseconds() / 1000.0f;
        m_Iterations++;
        if (m_Time > 0.0)
        {
            LOG("Iteration : %u, Time : %f, Iteration/sec : %f\n", m_Iterations, m_Time, m_Iterations / m_Time);
        }

        m_SphericalCameraController.updateCamera();
    }

    void keyboard(unsigned char key, int x, int y)
    {
        (void) x;
        (void) y;
        m_SphericalCameraController.onKeyboard(key, Controller::KEY_DOWN);
        if (key == 'f')
        {
           // TODO : m_Window->fullscreen();
        }
    }

    void keyboardUp(unsigned char key, int x, int y)
    {
        (void) x;
        (void) y;
        m_SphericalCameraController.onKeyboard(key, Controller::KEY_UP);
    }

    void mouse(int button, int state, int x, int y)
    {
        m_SphericalCameraController.mouse(button, state, x, y);
    }

    void motion(int x, int y)
    {
        m_SphericalCameraController.motion(x, y);
    }

    void special(int key, int x, int y)
    {
        (void) x;
        (void) y;
        m_SphericalCameraController.onSpecial(key, Controller::KEY_DOWN);
    }

    void specialUp(int key, int x, int y)
    {
        (void) x;
        (void) y;
        m_SphericalCameraController.onSpecial(key, Controller::KEY_UP);
    }

private:
    Clock m_Clock;
    OpenCL m_OpenCL;
    Camera m_Camera;
    SphericalCameraController m_SphericalCameraController;
    SphereMesh* m_Sphere;
    Shader::Program* m_Shader;

    std::vector<Node> m_Nodes;
    std::vector<Edge> m_Edges;

    unsigned long m_NumParticles;
    unsigned long m_NumForces;
    float m_K;
    float m_Time;
    unsigned int m_Iterations;
    float m_Temperature;

    OpenCL::CommandQueue* m_Queue;
    OpenCL::Kernel* m_RepulsionK;
    OpenCL::Kernel* m_AttractionK;
    OpenCL::Kernel* m_MovementK;
    OpenCL::Memory* m_InputNodeBuffer;
    OpenCL::Memory* m_InputEdgeBuffer;
    OpenCL::Memory* m_OutputNodeBuffer;
    OpenCL::Memory* m_ForceBuffer;
};

int main(int argc, char** argv)
{
    auto demo = new Raindance(argc, argv);
    demo->add(new DemoWindow("Particles", 1024, 728));
    demo->run();
    delete demo;
}
