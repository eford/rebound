using PyCall
@pyimport rebound
@pyimport ctypes
include("rebound_enum.jl")
include("rebound_constants.jl")    # Set astronomical constants

# Generate Initial Conditions
include("init_cond.jl")
target_mass = 5.* mass_earth
mulist = generate_embryo_mu_list(target_mass)

alpha = -1.5
min_a = 1.0
max_a = 10.0
alist = generate_embryo_draw_semimajor_axis_list(alpha,min_a,max_a,n=length(mulist))

# Setup Simulation, including (exagerated) radii
sim = rebound.Simulation()
sim[:add](m=1.,r=4*radius_sun)
for i in 1:length(alist)
   e = 1e-3*rand()
   inc = 0.5e-3*rand()
   omega = 2pi*rand()
   Omega = 2pi*rand()
   M = 2pi*rand()
   sim[:add](m=mulist[i],a=alist[i],e=e,inc=inc,omega=omega,Omega=Omega,M=M,r=4*radius_earth)
end
sim[:move_to_com]()
ptr_sim = convert(Ptr{Void},ctypes.addressof(sim)) # Get a pointer to simulation as a C struct

include("heartbeat.jl")      # Provide heartbeat function to print updates
activate_heartbeat(ptr_sim)  # Set simulation to use heartbeat_c

sim[:integrator] = REB_INTEGRATOR["IAS15"]
sim[:ri_ias15][:min_dt] = 1e-5*2pi     # Set minimum timestep for IAS15 integrator

# Set rebound to merge upon collisions
sim[:collision] = REB_COLLISION["DIRECT"]   # Set collision detection
librebound = Libdl.dlopen(LIB_REBOUND)
cptr_collision_merge = Libdl.dlsym(librebound,:reb_collision_resolve_merge)
#sim[:collisions_resolve] = cptr_collision_merge # Any reason not to do this directly?
ccall((:reb_set_collision_resolve,LIB_REBOUND),Void,(Ptr{Void},Ptr{Void},),ptr_sim,cptr_collision_merge) # Set simulation to use reb_collision_resolve_merge

# Turn on forces to induce a migration
ccall((:activate_migration_forces,LIB_REBOUND_USER),Void,(Ptr{Void},),ptr_sim)

# Set migration timescales
numpl = sim[:N]
for i in 2:numpl
  ccall((:set_tau_a,LIB_REBOUND_USER),Void,(Cint,Cdouble,),i,2000.0)
  ccall((:set_tau_e,LIB_REBOUND_USER),Void,(Cint,Cdouble,),i,2000.0)
end

# Run integration (and time how long it takes)
tend = 100*2pi
tic(); sim[:integrate](tend); toc()
sim[:status]()

sim[:save]("demo_6.bin")
