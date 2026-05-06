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
        float m_x, m_y, m_z;
        float m_yaw;
        float m_pitch;

        float m_speed;
        float m_sensitivity;
    };
}