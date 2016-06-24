using PyCall
@pyimport rebound

sim = rebound.Simulation()

# Setup initial conditions
sim[:add](m=1.)
sim[:add](m=0.001,a=1.0,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.3,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.6,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.9,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:move_to_com]()

# Run integration (and time how long it takes)
tend = 1000*2pi
tic(); sim[:integrate](tend); toc()
sim[:status]()

