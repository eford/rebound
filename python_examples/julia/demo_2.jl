const LIB_REBOUND_USER = Libdl.find_library(["librebounduser.so"],["."])
include("init_cond.jl")

using PyCall
@pyimport rebound

sim = rebound.Simulation()
sim[:add](m=1.)
for i in 1:length(alist)
   e = 1e-6*rand()
   inc = 0.5e-6*rand()
   omega = 2pi*rand()
   Omega = 2pi*rand()
   M = 2pi*rand()
   sim[:add](m=mulist[i],a=alist[i],e=e,inc=inc,omega=omega,Omega=Omega,M=M)
end

sim[:move_to_com]()
tend = 10*2pi

@time sim[:integrate](tend)

