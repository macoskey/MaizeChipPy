# Set of sample steering patterns
# J. Macoskey - May 2017

import numpy as np
import math as m
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

class FCC():
	
	def makePts(self):
		pts = np.empty((0,3))
		xmin,xmax = -self.xDim/2.0+self.x0,self.xDim/2.0+self.x0
		ymin,ymax = -self.yDim/2.0+self.y0,self.yDim/2.0+self.y0
		zmin,zmax = -self.zDim/2.0+self.z0,self.zDim/2.0+self.z0
		self.min_conds = np.array([xmin,ymin,zmin])
		self.max_conds = np.array([xmax,ymax,zmax])
		
		for x in np.arange(xmin,xmax+self.spacing*0.1,self.spacing):
			for y in np.arange(ymin,ymax+self.spacing*0.1,self.spacing):
				for z in np.arange(zmin,zmax+self.spacing*0.1,self.spacing):
					pts = np.append(pts,[[x,y,z]], axis=0)
					
		pts_dxy, pts_dyz, pts_dzx = np.zeros(pts.shape), np.zeros(pts.shape), np.zeros(pts.shape)
		pts_dxy[:,0:2] = -self.spacing/2.0
		pts_dyz[:,1:] = -self.spacing/2.0
		pts_dzx[:,0],pts_dzx[:,2] = -self.spacing/2.0,-self.spacing/2.0
		
		pts1 = np.vstack((pts,np.vstack((pts-pts_dxy,np.vstack((pts-pts_dyz,pts-pts_dzx))))))
		pts2 = np.vstack((pts1,np.vstack((pts+pts_dxy,np.vstack((pts+pts_dyz,pts+pts_dzx))))))
		
		b = np.ascontiguousarray(pts2).view(np.dtype((np.void, pts2.dtype.itemsize * pts2.shape[1])))
		
		_, idx = np.unique(b, return_index=True)

		self.pts = pts2[idx]
	
	def cube(self):
		self.makePts()
		for m in range(0,3):
			self.pts = self.pts[self.pts[:,m]>=self.min_conds[m],:]
			self.pts = self.pts[self.pts[:,m]<=self.max_conds[m],:]
		
	
	def sphere(self):
		self.makePts()
		pts = np.zeros(self.pts.shape)
		pts[:,0],pts[:,1],pts[:,2] = -self.x0,-self.y0,-self.z0
		
		pts+=self.pts
		r = (pts[:,0]**2+pts[:,1]**2+pts[:,2]**2)**0.5
		self.pts = self.pts[r<=self.radius,:]
	
	def __init__(self):
		self.spacing = 1
		self.radius = 5
		self.x0,self.y0,self.z0 = 0,0,0
		self.xDim, self.yDim, self.zDim = 10,10,10
		
	
class BCC():
	def makePts(self):
		pts = np.empty((0,3))
		xmin,xmax = -self.xDim/2.0+self.x0,self.xDim/2.0+self.x0
		ymin,ymax = -self.yDim/2.0+self.y0,self.yDim/2.0+self.y0
		zmin,zmax = -self.zDim/2.0+self.z0,self.zDim/2.0+self.z0
		self.min_conds = np.array([xmin,ymin,zmin])
		self.max_conds = np.array([xmax,ymax,zmax])
		
		for x in np.arange(xmin,xmax+self.spacing*0.1,self.spacing):
			for y in np.arange(ymin,ymax+self.spacing*0.1,self.spacing):
				for z in np.arange(zmin,zmax+self.spacing*0.1,self.spacing):
					pts = np.append(pts,[[x,y,z]], axis=0)
					
		pts_dxyz = np.zeros(pts.shape)
		pts_dxyz[:,0:] = -self.spacing/2.0
		
		pts2 = np.vstack((pts,np.vstack((pts-pts_dxyz,pts+pts_dxyz))))
		
		b = np.ascontiguousarray(pts2).view(np.dtype((np.void, pts2.dtype.itemsize * pts2.shape[1])))
		
		_, idx = np.unique(b, return_index=True)

		self.pts = pts2[idx]
	
	def cube(self):
		self.makePts()
		
		for m in range(0,3):
			self.pts = self.pts[self.pts[:,m]>=self.min_conds[m],:]
			self.pts = self.pts[self.pts[:,m]<=self.max_conds[m],:]
		
	
	def sphere(self):
		self.makePts()
		pts = np.zeros(self.pts.shape)
		pts[:,0],pts[:,1],pts[:,2] = -self.x0,-self.y0,-self.z0
		
		pts+=self.pts
		r = (pts[:,0]**2+pts[:,1]**2+pts[:,2]**2)**0.5
		self.pts = self.pts[r<=self.radius,:]
	
	def __init__(self):
		self.spacing = 1
		self.radius = 5
		self.x0,self.y0,self.z0 = 0,0,0
		self.xDim, self.yDim, self.zDim = 10,10,10
		

class HCP():
	
	def makePts(self):
		dx = self.spacing
		dy = (dx**2-(dx/2)**2)**0.5
		dz = 6.**0.5/3.*(dx)
		
		xmin,xmax = -self.xDim/2.0+self.x0,self.xDim/2.0+self.x0
		ymin,ymax = -self.yDim/2.0+self.y0,self.yDim/2.0+self.y0
		zmin,zmax = -self.zDim/2.0+self.z0,self.zDim/2.0+self.z0
		self.min_conds = np.array([xmin,ymin,zmin])
		self.max_conds = np.array([xmax,ymax,zmax])
		
		NX = int(np.ceil((xmax-xmin)/dx))
		NY = int(np.ceil((ymax-ymin)/dy))
		NZ = int(np.ceil((zmax-zmin)/dz))
		
		self.pts = np.empty((0,3))
		
		for nz in range(-int(np.ceil(NZ/2.0)),int(np.ceil(NZ/2.0))+1):
			for ny in range(-int(np.ceil(NY/2.0)),int(np.ceil(NY/2.0))+1):
				for nx in range(-int(np.ceil(NX/2.0)),int(np.ceil(NX/2.0))+1):
					
					if np.mod(ny,2) == 0:
						x = nx*dx+self.x0
					else:
						x = (nx+0.5)*dx+self.x0
					
					if np.mod(nz,2) == 0:
						y = ny*dy+self.y0
					else:
						y = (ny+(1.0-1./6.*3.0**0.5))*dy+self.y0
						
					z = nz*dz+self.z0
					
					self.pts = np.append(self.pts,[[x,y,z]],axis=0)

	
	def cube(self):
		self.makePts()
		
		for m in range(0,3):
			self.pts = self.pts[self.pts[:,m]>=self.min_conds[m],:]
			self.pts = self.pts[self.pts[:,m]<=self.max_conds[m],:]
		
	
	def sphere(self):
		self.makePts()
		pts = np.zeros(self.pts.shape)
		pts[:,0],pts[:,1],pts[:,2] = -self.x0,-self.y0,-self.z0
		
		pts+=self.pts
		r = (pts[:,0]**2+pts[:,1]**2+pts[:,2]**2)**0.5
		self.pts = self.pts[r<=self.radius,:]

	
	def __init__(self):
		self.spacing = 1
		self.radius = 2
		self.x0,self.y0,self.z0 = 0,0,0
		self.xDim, self.yDim, self.zDim = 10,10,10



x = HCP()
x.sphere()

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.scatter(x.pts[:,0],x.pts[:,1],x.pts[:,2], c='b',marker='o',s=20)
ax.set_ylabel('Y')
ax.set_xlabel('X')
plt.show()	

def makeCenter():
	pattern = np.array([[0,0,0],[0,0,0]])
	return pattern

def make4Points():
	pattern = np.array([[5,0,0],[0,5,0],[-5,0,0],[0,-5,0]])
	return pattern

def makeallthePoints():
	pattern = np.zeros((11**3,3))
	pattern[:,:] = 0
	g=0
	for i in range(-5,6):
		for j in range(-5,6):
			for k in range(-5,6):
				pattern[g,0],pattern[g,1],pattern[g,2] = i,j,k
				g+=1
	
	return pattern
	
def makeCube():
	
	return
	
def makeSphere():
	
	return
	
def makeCircle():
	
	return
	
def makeSpinner():
	
	return
