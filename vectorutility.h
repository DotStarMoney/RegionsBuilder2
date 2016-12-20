#ifndef VECTORUTILITY_H
#define VECTORUTILITY_H

#include <QVector2D>
#define VU_PI 3.14159265359

class VectorUtility
{
public:
    VectorUtility(){}
    static bool rotatedRectangleIntersection(QVector2D pos_a, QVector2D size_a, float angle_a,
                                             QVector2D pos_b, QVector2D size_b, float angle_b,
                                             float x[2][4], float y[2][4]){

        float radius1;
        float radius2;
        float angle1;
        float angle2;
        //float x[2][4];
        //float y[2][4];
        float axisx[4];
        float axisy[4];
        float proj;
        float minProj[2];
        float maxProj[2];
        int i, j, k;

        radius1 = sqrt(size_a.x()*size_a.x() + size_a.y()*size_a.y()) * 0.5;
        radius2 = sqrt(size_b.x()*size_b.x() + size_b.y()*size_b.y()) * 0.5;

        angle1 = asin((size_a.y() * 0.5) / radius1);
        angle2 = asin((size_b.y() * 0.5) / radius2);

        x[0][0] = pos_a.x() + radius1 * cos(angle_a - angle1);
        y[0][0] = pos_a.y() + radius1 * sin(angle_a - angle1);

        x[0][1] = pos_a.x() + radius1 * cos(angle_a + angle1);
        y[0][1] = pos_a.y() + radius1 * sin(angle_a + angle1);

        x[0][2] = pos_a.x() + radius1 * cos(angle_a + VU_PI - angle1);
        y[0][2] = pos_a.y() + radius1 * sin(angle_a + VU_PI - angle1);

        x[0][3] = pos_a.x() + radius1 * cos(angle_a + VU_PI + angle1);
        y[0][3] = pos_a.y() + radius1 * sin(angle_a + VU_PI + angle1);


        x[1][0] = pos_b.x() + radius2 * cos(angle_b - angle2);
        y[1][0] = pos_b.y() + radius2 * sin(angle_b - angle2);

        x[1][1] = pos_b.x() + radius2 * cos(angle_b + angle2);
        y[1][1] = pos_b.y() + radius2 * sin(angle_b + angle2);

        x[1][2] = pos_b.x() + radius2 * cos(angle_b + VU_PI - angle2);
        y[1][2] = pos_b.y() + radius2 * sin(angle_b + VU_PI - angle2);

        x[1][3] = pos_b.x() + radius2 * cos(angle_b + VU_PI + angle2);
        y[1][3] = pos_b.y() + radius2 * sin(angle_b + VU_PI + angle2);

        axisx[0] = x[0][0] - x[0][1];
        axisy[0] = y[0][0] - y[0][1];
        axisx[1] = x[0][2] - x[0][1];
        axisy[1] = y[0][2] - y[0][1];

        axisx[2] = x[1][0] - x[1][1];
        axisy[2] = y[1][0] - y[1][1];
        axisx[3] = x[1][2] - x[1][1];
        axisy[3] = y[1][2] - y[1][1];

        for(k = 0; k < 4; k++){

            proj = x[0][0] * axisx[k] + y[0][0] * axisy[k];
            minProj[0] = proj;
            maxProj[0] = proj;

            for(i = 1; i < 4; i++){
                proj = x[0][i] * axisx[k] + y[0][i] * axisy[k];

                if(proj < minProj[0])
                    minProj[0] = proj;
                else if(proj > maxProj[0])
                    maxProj[0] = proj;

            }

            proj = x[1][0] * axisx[k] + y[1][0] * axisy[k];
            minProj[1] = proj;
            maxProj[1] = proj;

            for(j = 1; j < 4; j++){
                proj = x[1][j] * axisx[k] + y[1][j] * axisy[k];

                if(proj < minProj[1])
                    minProj[1] = proj;
                else if(proj > maxProj[1])
                    maxProj[1] = proj;

            }

            if((maxProj[1] < minProj[0]) || (maxProj[0] < minProj[1])) return false;
       }

       return true;
    }

};

#endif // VECTORUTILITY_H
