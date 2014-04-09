namespace octet {
/*  Subroutine to generate B-spline basis functions for open knot vectors

	C code for An Introduction to NURBS
	by David F. Rogers. Copyright (C) 2000 David F. Rogers,
	All rights reserved.
	
	Name: basis.c
	Language: C
	Subroutines called: none
	Book reference: p. 279

    c        = order of the B-spline basis function
    d        = first term of the basis function recursion relation
    e        = second term of the basis function recursion relation
    npts     = number of defining polygon vertices
    n[]      = array containing the basis functions
               n[1] contains the basis function associated with B1 etc.
    nplusc   = constant -- npts + c -- maximum number of knot values
    t        = parameter value
    temp[]   = temporary array
		x[]      = knot vector
		*/	

	void get_basis_functions(int c, float t, int npts,float *x, float *n)
	{
		int nplusc;
		float d,e;
		float temp[36];

		nplusc = npts + c + 1;

		/*		printf("knot vector is \n");
					for (i = 1; i <= nplusc; i++){
					printf(" %d %d \n", i,x[i]);
					}
					printf("t is %f \n", t);
					*/

		/* calculate the first order basis functions n[i][1]	*/

		for (int i = 0; i < nplusc-1; i++){
			if (( t >= x[i]) && (t < x[i+1]))
				temp[i] = 1;
			else
				temp[i] = 0;
		}

		/* calculate the higher order basis functions */

		for (int k = 1; k <= c; k++){
			for (int i = 0; i < nplusc-k; i++){
				if (temp[i] != 0)    /* if the lower order basis function is zero skip the calculation */
					d = ((t-x[i])*temp[i])/(x[i+k]-x[i]);
				else
					d = 0;

				if (temp[i+1] != 0)     /* if the lower order basis function is zero skip the calculation */
					e = ((x[i+k+1]-t)*temp[i+1])/(x[i+k+1]-x[i+1]);
				else
					e = 0;

				temp[i] = d + e;
			}
		}

		if (t == x[nplusc - 1]){		//    pick up last point
			temp[npts - 1] = 1;
		}

		/* put in n array	*/

		for(int i = 0; i < npts; i++) {
			n[i] = temp[i];
		}
	}

	void get_surface_vertex(float *vertex, float *ctrl_points, float *basis_u, float u_count, float *basis_v, float v_count)
	{
		int index = 0, index1 = 0;
		float vertices_v[1000] = {0};
		for(int i = 0; i < v_count; i++)
		{
			for(int j = 0; j < u_count; j++)
			{
				vertices_v[index1] += basis_u[j] * ctrl_points[index];
				vertices_v[index1 + 1] += basis_u[j] * ctrl_points[index + 1];
				vertices_v[index1 + 2] += basis_u[j] * ctrl_points[index + 2];
				index += 3; 
			}
			index1 += 3;
		}
		index = 0;
		vertex[0] = vertex[1] = vertex[2] = 0;
		for(int k = 0; k < v_count; k++)
		{
			vertex[0] += basis_v[k] * vertices_v[index];
			vertex[1] += basis_v[k] * vertices_v[index + 1];
			vertex[2] += basis_v[k] * vertices_v[index + 2];
			index += 3; 
		}
	}

}
