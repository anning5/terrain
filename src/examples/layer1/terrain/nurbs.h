namespace octet {

	class nurbs_surface
	{
		dynarray<vec3> ctrl_points;
		dynarray<float> knots_u;
		dynarray<float> knots_v;
		dynarray<float> basis_u;
		dynarray<float> basis_v;
		int degree_u;
		int degree_v;
	public:

		nurbs_surface() :
			degree_u(0),
			degree_v(0)
		{
			//float knots[] = {0, 0, 0 ,0, 1, 1, 1, 1};
			//float knots[] = {0, 0, 0 ,0, .1f, .3f, .6f, 1};
		}

		void get_surface_vertex(vec3 &vertex, float u, float v)
		{
			get_basis_functions(degree_u, u, knots_u, basis_u);
			get_basis_functions(degree_v, v, knots_v, basis_v);
			int u_count = 4, v_count = 4;
			int index = 0;
			dynarray<vec3> vertices_v;
			vertices_v.resize(v_count);
			for(int i = 0; i < v_count; i++)
			{
				vertices_v[i] = vec3(0, 0, 0);
				for(int j = 0; j < u_count; j++)
				{
					vertices_v[i] += basis_u[j] * ctrl_points[index];
					index++;
				}
			}
			vertex = vec3(0, 0, 0);
			for(int k = 0; k < v_count; k++)
			{
				vertex += basis_v[k] * vertices_v[k];
			}
		}

		void set_knot_u(unsigned int index, float k)
		{
			if(index >= knots_u.size())
			{
				return;
			}
			knots_u[index] = k;
		}

		void set_knot_v(unsigned int index, float k)
		{
			if(index >= knots_v.size())
			{
				return;
			}
			knots_v[index] = k;
		}

		void add_knot_v(float k)
		{
			knots_v.push_back(k);
			if(knots_v.size() > 4)
			{
				basis_v.resize(knots_v.size() - 4);
			}
		}

		void add_knot_u(float k)
		{
			knots_u.push_back(k);
			if(knots_u.size() > 4)
			{
				basis_u.resize(knots_u.size() - 4);
			}
		}

		void set_degree_v(int d)
		{
			degree_v = d;
		}
		
		void set_degree_u(int d)
		{
			degree_u = d;
		}
		
		void reset()
		{
			basis_u.reset();
			basis_v.reset();
			knots_u.reset();
			knots_v.reset();
			ctrl_points.reset();
		}

		const dynarray<vec3> &get_ctrl_points()
		{
			return ctrl_points;
		}

		void add_ctrl_points(const vec3 &v)
		{
			ctrl_points.push_back(v);
		}

		void set_ctrl_points(unsigned int index, const vec3 &v)
		{
			if(index >= ctrl_points.size())
			{
				return;
			}
			ctrl_points[index] = v;
		}

		vec3 *get_ctrl_points(unsigned int index)
		{
			if(index >= ctrl_points.size())
			{
				return NULL;
			}
			return &ctrl_points[index];
		}

		void get_basis_functions(int degree, float t, const dynarray<float> &knots, dynarray<float> &basis)
		{
			/*
			*/	
			unsigned int knot_count;
			unsigned int ctrl_point_count = basis.size();
			float d,e;
			float temp[36];

			knot_count = knots.size();

			for (unsigned int i = 0; i < knot_count - 1; i++){
				if (( t >= knots[i]) && (t < knots[i + 1]))
					temp[i] = 1;
				else
					temp[i] = 0;
			}

			/* calculate the higher order basis functions */

			for (int k = 1; k <= degree; k++){
				for (unsigned int i = 0; i < knot_count - k; i++){
					if (temp[i] != 0)    /* if the lower order basis function is zero skip the calculation */
						d = ((t-knots[i])*temp[i])/(knots[i + k]-knots[i]);
					else
						d = 0;

					if (temp[i + 1] != 0)     /* if the lower order basis function is zero skip the calculation */
						e = ((knots[i + k+1]-t)*temp[i + 1])/(knots[i + k + 1]-knots[i + 1]);
					else
						e = 0;

					temp[i] = d + e;
				}
			}

			if (t == knots[knot_count - 1] || t > knots[knot_count - 1] && (t - knots[knot_count - 1]) < .00001 ){		//    pick up last point
				temp[ctrl_point_count - 1] = 1;
			}

			/* put in n array	*/

			for(unsigned int i = 0; i < ctrl_point_count; i++) {
				basis[i] = temp[i];
			}
		}


		~nurbs_surface()
		{
		}
	};

}
