using PyCall
@pyimport rebound
@pyimport ctypes
include("rebound_constants.jl")    # Set astronomical constants

# Setup Simulation, now including (exagerated) radii
sim = rebound.Simulation()
sim[:add](m=1.,r=4*radius_sun)
sim[:add](m=10*mass_earth,a=1.0,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand(),r=radius_jupiter)
sim[:add](m=10*mass_earth,a=1.03,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand(),r=radius_jupiter)
sim[:add](m=10*mass_earth,a=1.06,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand(),r=radius_jupiter)
sim[:add](m=10*mass_earth,a=1.09,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand(),r=radius_jupiter)
sim[:move_to_com]()
ptr_sim = convert(Ptr{Void},ctypes.addressof(sim)) # Get a pointer to simulation as a C struct

include("heartbeat.jl")      # Provide heartbeat function to print updates
activate_heartbeat(ptr_sim)  # Set simulation to use heartbeat_c

include("rebound_enum.jl")                  # Load enum equivalents
sim[:integrator] = REB_INTEGRATOR["IAS15"]  # Set integrator (to default)
#sim[:integrator] = REB_INTEGRATOR["WHFAST"]  # Set integrator, alternatives (not appropriate for collisions)
#sim[:integrator] = REB_INTEGRATOR["HERMES"]  # Set integrator, alternatives (experimental)

# Set rebound to merge upon collisions
sim[:collision] = REB_COLLISION["DIRECT"]   # Set collision detection
librebound = Libdl.dlopen(LIB_REBOUND)      # Open librebound 
cptr_collision_merge = Libdl.dlsym(librebound,:reb_collision_resolve_merge) # Get C pointer to function to merge bodies
#sim[:collisions_resolve] = cptr_collision_merge  # Any reason not to do this directly?
ccall((:reb_set_collision_resolve,LIB_REBOUND),Void,(Ptr{Void},Ptr{Void},),ptr_sim,cptr_collision_merge) # Set simulation to use reb_collision_resolve_merge  

# Run integration (and time how long it takes)
tend = 10000*2pi
tic(); sim[:integrate](tend); toc()
sim[:status]()

