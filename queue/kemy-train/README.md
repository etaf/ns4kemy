# Kemy : An AQM generator based on machine learning.

--------
Kemy is an offline machine learning algorithm.
This project is the training code of Kemy.

--------

Before run the training code, please make sure the following tools have been correctly installed in you running environment.
>**Required tools:**

* C++ 11 compiler (we use gcc 4.8)
* [Google Protocol Buffers](https://code.google.com/p/protobuf/)
* [Boost C++ libraries](http://www.boost.org/)

After the required tools have been installed, please follow the step below to reproduce the result.
>**steps:**

 1. ``` git clone https://github.com/jkemy/Kemy-training.git ```
 2. ``` tar -xf kemy.tar.gz ```
 3. ``` cd kemy-clear ```
 4. ``` ./autogen.sh ```
 5. ``` ./configure ```
 6. ``` make ```
 7. ``` cd src ```
 8. if successful,you can run kemy now:
    ```./kemy ```

