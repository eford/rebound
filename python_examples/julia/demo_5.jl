using PyCall
@pyimport rebound
@pyimport ctypes
include("rebound_enum.jl")
include("rebound_constants.jl")  

# Setup Simulation, including less exagerated radii
sim = rebound.Simulation()
sim[:add](m=1.,r=4*radius_sun)
sim[:add](m=10*mass_earth,a=1.0,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand(),r=4*radius_earth)
sim[:add](m=10*mass_earth,a=1.03,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand(),r=4*radius_earth)
sim[:add](m=10*mass_earth,a=1.06,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand(),r=4*radius_earth)
sim[:add](m=10*mass_earth,a=1.09,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand(),r=4*radius_earth)
sim[:move_to_com]()
ptr_sim = convert(Ptr{Void},ctypes.addressof(sim)) # Get a pointer to simulation as a C struct

include("heartbeat.jl")      # Provide heartbeat function to print updates
activate_heartbeat(ptr_sim)  # Set simulation to use heartbeat_c

sim[:integrator] = REB_INTEGRATOR["IAS15"]

# Set rebound to merge upon collisions
sim[:collision] = REB_COLLISION["DIRECT"]
librebound = Libdl.dlopen(LIB_REBOUND)
cptr_collision_merge = Libdl.dlsym(librebound,:reb_collision_resolve_merge)
#sim[:collisions_resolve] = cptr_collision_merge 
ccall((:reb_set_collision_resolve,LIB_REBOUND),Void,(Ptr{Void},Ptr{Void},),ptr_sim,cptr_collision_merge)

# Turn on forces to induce a migration
ccall((:activate_migration_forces,LIB_REBOUND_USER),Void,(Ptr{Void},),ptr_sim)

# Set migration timescales
numpl = sim[:N]
for i in 2:numpl
  ccall((:set_tau_a,LIB_REBOUND_USER),Void,(Cint,Cdouble,),i,0.0)
  ccall((:set_tau_e,LIB_REBOUND_USER),Void,(Cint,Cdouble,),i,0.0)
end
ccall((:set_tau_a,LIB_REBOUND_USER),Void,(Cint,Cdouble,),numpl,2000.0)
ccall((:set_tau_e,LIB_REBOUND_USER),Void,(Cint,Cdouble,),numpl,200.0)

# Run integration (and time how long it takes)
tend = 10000*2pi
tic(); sim[:integrate](tend); toc()
sim[:status]()
