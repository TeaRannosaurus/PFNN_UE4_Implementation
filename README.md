# Project currently on hold. It might continue in the future but for now focus lies elsewhere

# Phased function neural network unreal implementation
Basic implementation of the phased function neural network in Unreal Engine 4.
This project is intended for the 2018/2019 Year3 Survival game at Breda University of Applied Sciences.

# Project state
As of right now the implementation is not working as intended. Some math needs to be debugged and solved before the intended results are present. 
## Points that need fixing/implementing
* Aply to mesh
  * Model transforms and bones not being set correctly at the moment
* Trajectory
  * Input passed to trajectory is incorrect
  * Trajectory is not on possesed pawn location
* Crouching is being skipped for now
* Jumping is being skipped for now

# Trained network
A fully trained PFNN is included into the project. The trained NN is loaded once you hit the play button. The location of this NN is in Content/MachineLearning/PhaseFunctionNeuralNetwork
Going in the Weights folder the weights and biasses of the network are stored. They are in a binary format. 

# Third party software
In this unreal project the following thirdparty mathematical libraries are used:
* Eigen3  http://eigen.tuxfamily.org/index.php?title=Main_Page
* GLM     https://glm.g-truc.net/0.9.9/index.html

# Resources
Implementation is mainly derived from the following research paper: http://theorangeduck.com/media/uploads/other_stuff/phasefunction.pdf
