using PyCall
@pyimport rebound
@pyimport ctypes             # For getting C pointers from Python objects

# Setup Simulation
sim = rebound.Simulation()
sim[:add](m=1.)
sim[:add](m=0.001,a=1.0,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.3,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.6,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.9,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:move_to_com]()

# Get a pointer to simulation as a C struct  
ptr_sim = convert(Ptr{Void},ctypes.addressof(sim))  

# Provides heartbeat_jl function to print updates
# Provides heartbeat_c, a pointer to a C-callable function pointer to heartbeat_jl
include("heartbeat.jl")     
activate_heartbeat(ptr_sim)

# Run integration (and time how long it takes)
tend = 1000*2pi
tic(); sim[:integrate](tend); toc()
sim[:status]()

