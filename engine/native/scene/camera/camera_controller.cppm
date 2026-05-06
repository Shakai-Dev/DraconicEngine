export module scene.camera.controller;

import rendering.renderer;

export namespace draco::scene
{
    struct CameraController
    {
        void init(float x = 0.0f, float y = 0.0f, float z = -2.0f);

        void update(float dt);

        draco::rendering::renderer::Camera get_camera() const;

    private:
        // Init with default values
        float m_x = 0.0f, m_y = 0.0f, m_z = 0.0f;
        float m_yaw = 0.0f;
        float m_pitch = 0.0f;
        float m_speed = 5.0f;
        float m_sensitivity = 0.1f;
    };
}