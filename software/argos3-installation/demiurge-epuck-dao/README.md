# README
demiurge-epuck-dao
=====================

This package contains the reference models of the e-puck robot
used mainly in AutoMoDe methods.
It acts as a *Data Access Object* for all low level actuation
and sensing of the epuck robot.
All reference models inherit from the EpuckDAO class.

All useful information about the AutoMoDe package, including
installation and utilization instructions, are regrouped in the
following technical report ([techrep](#bibliography)). Please cite this report if you use any ARGoS3-AutoMoDe related package.

## Package content
- `src/` contains the source files of all reference models.
    - `EpuckDAO.*` contain the mother class from which all other
        reference models are derived.
    - `ReferenceModel<a>Dot<b>.*` contains the source of reference
        model version a.b (see [refmodel](#bibliography) for
        more information about the different reference models)

## Installation
### Dependencies
- [ARGoS3](https://github.com/ilpincy/argos3) (3.0.0-beta48)
- [argos3-epuck](https://github.com/demiurge-project/argos3-epuck) (v48)

### Compiling and installing
    $ git clone https://github.com/demiurge-project/demiurge-epuck-dao.git
    $ cd demiurge-epuck-dao
    $ mkdir build
    $ cmake ..
    $ make
    $ sudo make install

Once compiled and installed the shared library and header files
are installed in the system.

If you do not have root access off what to install in a your local
argos distribution folder use (if your argos installation is located
in `~/argos-dist`):

    $ cmake .. -DCMAKE_INSTALL_PREFIX=~/argos-dist
    $ make
    $ make install

### How to use
This package is meant to be used as a library to allow unified access
to actuators and sensors for different automatic design methods.
It is used in AutoMoDe, Evostick and other automatic design methods
(see [references](#bibliography)).



## References
### Bibliography

- [refmodel] Hasselmann, K., Ligot, A., Francesca, G., & Birattari, M. (2018). Reference models for AutoMoDe. Technical report TR/IRIDIA/2018-002, IRIDIA, Université libre de Bruxelles, Belgium.
- [techrep] Ligot, A., Hasselmann, K., Delhaisse, B., Garattoni, L., Francesca, G., & Birattari, M. (2017). AutoMoDe, NEAT, and EvoStick: implementations for the E-puck robot in ARGoS3. Technical report TR/IRIDIA/2017-002, IRIDIA, Université libre de Bruxelles, Belgium.
- [chocolate] Francesca, G., Brambilla, M., Brutschy, A., Garattoni, L., Miletitch, R., Podevijn, G., ... & Mascia, F. (2015). AutoMoDe-Chocolate: automatic design of control software for robot swarms. Swarm Intelligence, 9(2-3), 125-152.
- [gianduja] Hasselmann K., Robert F., Birattari M. (2018) Automatic Design of Communication-Based Behaviors for Robot Swarms. In: Dorigo M., Birattari M., Blum C., Christensen A., Reina A., Trianni V. (eds) Swarm Intelligence. ANTS 2018. Lecture Notes in Computer Science, vol 11172. Springer, Cham
