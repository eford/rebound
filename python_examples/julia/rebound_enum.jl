# Enumeration describing the return status of rebound_integrate
REB_STATUS = Dict{ASCIIString,Cint}(
    "REB_RUNNING_PAUSED" => -3,    # ///< Simulation is paused by visualization.
    "REB_RUNNING_LAST_STEP" => -2, # ///< Current timestep is the last one. Needed to ensures that t=tmax exactly.
    "REB_RUNNING" => -1,           # ///< Simulation is current running, no error occured.
    "REB_EXIT_SUCCESS" => 0,       # ///< Integration finished successfully.
    "REB_EXIT_ERROR" => 1,     # ///< A generic error occured and the integration was not successfull.
    "REB_EXIT_NOPARTICLES" => 2,   # ///< The integration ends early because no particles are left in the simulation.
    "REB_EXIT_ENCOUNTER" => 3,     # ///< The integration ends early because two particles had a close encounter (see exit_min_distance)
    "REB_EXIT_ESCAPE" => 4,        # ///< The integration ends early because a particle escaped (see exit_max_distance)  
    "REB_EXIT_USER" => 5)      # ///< User caused exit, simulation did not finish successfully.

# Available collision routines
REB_COLLISION = Dict{ASCIIString,Cint}(
        "NONE" => 0,     # ///< Do not search for collisions (default)
        "DIRECT" => 1,   # ///< Direct collision search O(N^2)
        "TREE" => 2)     # ///< Tree based collision search O(N log(N))

# Available integrators
REB_INTEGRATOR = Dict{ASCIIString,Cint}(
        "IAS15" => 0,   # ///< IAS15 integrator, 15th order, non-symplectic (default)
        "WHFAST" => 1,  # ///< WHFast integrator, symplectic, 2nd order, up to 11th order correctors
        "SEI" => 2,     # ///< SEI integrator for shearing sheet simulations, symplectic, needs OMEGA variable
        "WH" => 3,      # ///< WH integrator (based on swifter), WHFast is recommended, this integrator is in REBOUND for comparison tests only
        "LEAPFROG" => 4,#  ///< LEAPFROG integrator, simple, 2nd order, symplectic
        "HERMES" => 5,  # ///< HERMES Integrator for close encounters (experimental)
        "NONE" => 6)    #  ///< Do not integrate anything


# Available boundary conditions
REB_BOUNDARY = Dict{ASCIIString,Cint}(
        "NONE" => 0,     # ///< Do not check for anything (default)
        "OPEN" => 1,     # ///< Open boundary conditions. Removes particles if they leave the box 
        "PERIODIC" => 2, # ///< Periodic boundary conditions
        "SHEAR" => 3)    # ///< Shear periodic boundary conditions, needs OMEGA variable

# 
REB_GRAVITY = Dict{ASCIIString,Cint}(
        "NONE" => 0,       #///< Do not calculate graviational forces
        "BASIC" => 1,      # ///< Basic O(N^2) direct summation algorithm, choose this for shearing sheet and periodic boundary conditions
        "COMPENSATED" => 2,#    ///< Direct summation algorithm O(N^2) but with compensated summation, slightly slower than BASIC but more accurate
        "TREE" => 3);       # ///< Use the tree to calculate gravity, O(N log(N)), set opening_angle2 to adjust accuracy.

