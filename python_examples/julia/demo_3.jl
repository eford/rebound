const LIB_REBOUND = Libdl.find_library(["librebound.so"],["."])
const LIB_REBOUND_USER = Libdl.find_library(["librebounduser.so"],["."])

using PyCall
@pyimport rebound
@pyimport ctypes

sim = rebound.Simulation()
sim[:add](m=1.)
sim[:add](m=0.001,a=1.0,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.3,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.6,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.9,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:move_to_com]()

function heartbeat_jl(reb::Ptr{Void})
   tmax	= 2.0e3*2pi
   retval::Cint = ccall((:reb_output_check,LIB_REBOUND),Cint,(Ptr{Void},Cdouble,),reb,2*pi)
   if retval>0
     ccall((:reb_output_timing,LIB_REBOUND),Cint,(Ptr{Void},Cdouble,),reb,tmax)
   end	  
   convert(Cint,0)
end 
const heartbeat_c = cfunction(heartbeat_jl,Cint,(Ptr{Void},) )

psim = convert(Ptr{Void},ctypes.addressof(sim))
ccall((:set_heartbeat,LIB_REBOUND_USER),Void,(Ptr{Void},Ptr{Void},),psim,heartbeat_c)

tend = 100*2pi
@time sim[:integrate](tend)


#=
	if(reb_output_check(r, 200.*M_PI)){
		reb_output_timing(r, tmax);
	}
	if(reb_output_check(r, 400.*M_PI)){
		reb_integrator_synchronize(r);
		reb_output_orbits(r,"orbits.txt");
		reb_move_to_com(r); 
	}
=#

#=
p = convert(Ptr{Void},ctypes.addressof(sim))
ccall((:activate_migration_forces,LIB_REBOUND_USER),Void,(Ptr{Void},),p)

ccall((:set_tau_a,LIB_REBOUND_USER),Void,(Cint,Cdouble,),2,0.001)

ccall((:get_tau_a,LIB_REBOUND_USER),Cdouble,(Cint,),1)
=#

#=

pysim = convert(PyObject,sim)
 
function migration_forces_julia(sim::PyObject)
  ccall((:migration_forces,LIB_REBOUND_USER),Void,(Ptr{Void},),sim)
end
 
migration_forces_c_ptr = cfunction(migration_forces_julia,Void,(Ptr{UInt8},))
=#


