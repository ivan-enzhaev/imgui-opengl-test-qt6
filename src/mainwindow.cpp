#include "mainwindow.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

// Helper to translate Qt Key to ImGuiKey
static ImGuiKey QtKeyToImGuiKey(int key)
{
    switch (key)
    {
    case Qt::Key_Tab:
        return ImGuiKey_Tab;
    case Qt::Key_Left:
        return ImGuiKey_LeftArrow;
    case Qt::Key_Right:
        return ImGuiKey_RightArrow;
    case Qt::Key_Up:
        return ImGuiKey_UpArrow;
    case Qt::Key_Down:
        return ImGuiKey_DownArrow;
    case Qt::Key_PageUp:
        return ImGuiKey_PageUp;
    case Qt::Key_PageDown:
        return ImGuiKey_PageDown;
    case Qt::Key_Home:
        return ImGuiKey_Home;
    case Qt::Key_End:
        return ImGuiKey_End;
    case Qt::Key_Insert:
        return ImGuiKey_Insert;
    case Qt::Key_Delete:
        return ImGuiKey_Delete;
    case Qt::Key_Backspace:
        return ImGuiKey_Backspace;
    case Qt::Key_Space:
        return ImGuiKey_Space;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        return ImGuiKey_Enter;
    case Qt::Key_Escape:
        return ImGuiKey_Escape;
    case Qt::Key_Control:
        return ImGuiKey_LeftCtrl;
    case Qt::Key_Shift:
        return ImGuiKey_LeftShift;
    case Qt::Key_Alt:
        return ImGuiKey_LeftAlt;
    default:
        if (key >= Qt::Key_A && key <= Qt::Key_Z)
            return (ImGuiKey)(ImGuiKey_A + (key - Qt::Key_A));
        if (key >= Qt::Key_0 && key <= Qt::Key_9)
            return (ImGuiKey)(ImGuiKey_0 + (key - Qt::Key_0));
        return ImGuiKey_None;
    }
}

// --- Vertex & Fragment Shaders ---
static const char *vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;
void main() {
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

static const char *fragmentShaderSource = R"(#version 330 core
in vec3 ourColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(ourColor, 1.0);
}
)";

// ==========================================
// GLWidget Implementation
// ==========================================
GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget()
{
    makeCurrent();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)
        glDeleteBuffers(1, &m_vbo);
    if (m_shaderProgram)
        glDeleteProgram(m_shaderProgram);
    doneCurrent();
}

void GLWidget::initializeGL()
{
    // Initialize GLAD with Qt's proc loader
    bool ok = gladLoadGLLoader([](const char *name) -> void * {
        return reinterpret_cast<void *>(QOpenGLContext::currentContext()->getProcAddress(name));
    });

    if (!ok)
    {
        qFatal("Failed to initialize GLAD using Qt OpenGL context!");
    }

    // 1. Shaders & Triangle
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fs);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vs);
    glAttachShader(m_shaderProgram, fs);
    glLinkProgram(m_shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    float vertices[] = {
        0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Dear ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 330");

    m_timer.start();
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void GLWidget::paintGL()
{
    // Draw 3D OpenGL scene
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_shaderProgram);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    // Render ImGui UI on top
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width(), (float)height());
    float dpr = (float)devicePixelRatio();
    io.DisplayFramebufferScale = ImVec2(dpr, dpr);

    float dt = (float)m_timer.restart() / 1000.0f;
    io.DeltaTime = dt > 0.0f ? dt : (1.0f / 60.0f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // ==============================================================
    // SET IMGUI WINDOW POSITION & SIZE HERE (BEFORE CALLING WINDOW)
    // ==============================================================
    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320.0f, 220.0f), ImGuiCond_FirstUseEver);

    ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Input handling
void GLWidget::mousePressEvent(QMouseEvent *event)
{
    int button = -1;
    if (event->button() == Qt::LeftButton)
        button = 0;
    if (event->button() == Qt::RightButton)
        button = 1;
    if (event->button() == Qt::MiddleButton)
        button = 2;

    if (button != -1)
        ImGui::GetIO().AddMouseButtonEvent(button, true);

    update();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int button = -1;
    if (event->button() == Qt::LeftButton)
        button = 0;
    if (event->button() == Qt::RightButton)
        button = 1;
    if (event->button() == Qt::MiddleButton)
        button = 2;

    if (button != -1)
        ImGui::GetIO().AddMouseButtonEvent(button, false);

    update();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    ImGui::GetIO().AddMousePosEvent((float)event->position().x(), (float)event->position().y());
    update();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    if (!numPixels.isNull())
    {
        ImGui::GetIO().AddMouseWheelEvent((float)numPixels.x() * 0.1f, (float)numPixels.y() * 0.1f);
    }
    else if (!numDegrees.isNull())
    {
        ImGui::GetIO().AddMouseWheelEvent((float)numDegrees.x() / 15.0f, (float)numDegrees.y() / 15.0f);
    }
    update();
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    ImGuiKey imguiKey = QtKeyToImGuiKey(event->key());
    if (imguiKey != ImGuiKey_None)
    {
        ImGui::GetIO().AddKeyEvent(imguiKey, true);
    }
    if (!event->text().isEmpty())
    {
        ImGui::GetIO().AddInputCharactersUTF8(event->text().toUtf8().constData());
    }
    update();
}

void GLWidget::keyReleaseEvent(QKeyEvent *event)
{
    ImGuiKey imguiKey = QtKeyToImGuiKey(event->key());
    if (imguiKey != ImGuiKey_None)
    {
        ImGui::GetIO().AddKeyEvent(imguiKey, false);
    }
    update();
}

// ==========================================
// MainWindow Implementation
// ==========================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt 6 + ImGui 1.93 + OpenGL 3.3 Core");
    resize(800, 600);

    m_glWidget = new GLWidget(this);
    setCentralWidget(m_glWidget);
}
