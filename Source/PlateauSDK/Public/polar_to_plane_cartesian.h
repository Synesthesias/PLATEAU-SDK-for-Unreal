#pragma once
class polar_to_plane_cartesian {
public:
    void convert(double xyz[]);
private:
    double Merid(double phi2);

    int jt_ = 5;
    double n_ = 0, anh_ = 0;
};