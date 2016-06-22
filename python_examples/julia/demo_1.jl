using PyCall
@pyimport rebound

sim = rebound.Simulation()
sim[:add](m=1.)
sim[:add](m=0.001,a=1.0,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:move_to_com]()

tend = 10*2pi
@time sim[:integrate](tend)


