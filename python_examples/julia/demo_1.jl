using PyCall                 # Use Julia package for calling Python packages
@pyimport rebound            # Import rebound Python interface

sim = rebound.Simulation()   # Create a simulation object

# Setup initial conditions
sim[:add](m=1.)
sim[:add](m=0.001,a=1.0,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:move_to_com]() # Shift frame so center-of-mass is at rest

# Integrate simulation object
tend = 10*2pi
sim[:integrate](tend)
sim[:status]()
