#pragma once

struct CameraController;

struct ObjectDetails {
    ObjectDetails();

    // FIXME: Avoid passing the camera controller to this function
    void render(CameraController&);
};
