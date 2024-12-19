#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WGS84_A (6378.137) /* In km */
#define WGS84_E_SQUARED (6.69437999014E-3)
#define WGS84_OMEGA (7.2921159E-5) /* Earth angular velocity in rad/s */

#define J2000 (946728000.0) /* J2000 is the epoch at 1/1/2000 12:00 noon. This constant is the epoch
                               in UNIX seconds */
#define EARTH_ANGLE_AT_J2000 (280.46) /* The rotational angle of the earth at J2000, in degrees */

#endif
