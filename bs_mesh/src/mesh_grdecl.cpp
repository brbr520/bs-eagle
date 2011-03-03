/*! \file mesh_grdecl.cpp
\brief This file implement class for working with grid_ecllipse
\author Iskhakov Ruslan
\date 2008-05-20 */
#include "bs_mesh_stdafx.h"
#include "mesh_grdecl.h"

#define BOUND_MERGE_THRESHOLD 0.8

using namespace grd_ecl;
using namespace blue_sky;

const char filename_hdf5[] = "grid_swap.h5";

#ifdef BS_MESH_WRITE_TRANSMISS_MATRIX  
  FILE*  fp;
#endif //BS_MESH_WRITE_TRANSMISS_MATRIX 

template< class strategy_t >
struct mesh_grdecl< strategy_t >::inner {
	// shorter aliases
	typedef i_type_t int_t;
	typedef fp_type_t fp_t;
	typedef fp_storage_type_t fp_stor_t;

	// other typedefs
	typedef bs_array< fp_stor_t, bs_vector_shared > fp_storvec_t;
	typedef bs_array< fp_stor_t, bs_array_shared > fp_storarr_t;
	typedef smart_ptr< fp_storvec_t > spfp_storvec_t;
	typedef smart_ptr< fp_storarr_t > spfp_storarr_t;
	typedef std::pair< spfp_storarr_t, spfp_storarr_t > coord_zcorn_pair;
	typedef std::set< fp_t > fp_set;

	typedef typename fp_storvec_t::iterator v_iterator;
	typedef typename fp_storvec_t::const_iterator cv_iterator;
	typedef typename fp_set::iterator fps_iterator;

	// iterator that jumps over given offset instead of fixed +1
	template< class iterator_t, int step_size = 1 >
	class slice_iterator : public std::iterator<
							typename std::iterator_traits< iterator_t >::iterator_category,
							typename std::iterator_traits< iterator_t >::value_type,
							typename std::iterator_traits< iterator_t >::difference_type,
							typename std::iterator_traits< iterator_t >::pointer,
							typename std::iterator_traits< iterator_t >::reference
							>
	{
		typedef std::iterator_traits< iterator_t > traits_t;

	public:
		typedef typename traits_t::value_type value_type;
		typedef typename traits_t::pointer pointer;
		typedef typename traits_t::reference reference;
		typedef typename traits_t::difference_type difference_type;

		// set step in compile-time
		slice_iterator() : p_(), step_(step_size) {}
		slice_iterator(const iterator_t& i) : p_(i), step_(step_size) {}
		slice_iterator(const slice_iterator& i) : p_(i.p_), step_(i.step_) {}

		// set step in runtime
		slice_iterator(difference_type step) : p_(), step_(step) {}
		slice_iterator(const iterator_t& i, difference_type step) : p_(i), step_(step) {}

		reference operator[](difference_type i) const {
			return p_[i * step_];
		}

		void operator+=(difference_type n) {
			p_ += n * step_;
		}
		void operator-=(difference_type n) {
			p_ -= n * step_;
		}

		friend slice_iterator operator+(const slice_iterator& lhs, difference_type n) {
			return lhs.p_ + (n * lhs.step_);
		}
		friend slice_iterator operator+(difference_type n, const slice_iterator& lhs) {
			return lhs.p_ + (n * lhs.step_);
		}
		friend slice_iterator operator-(const slice_iterator& lhs, difference_type n) {
			return lhs.p_ - (n * lhs.step_);
		}

		slice_iterator& operator++() {
			p_ += step_;
			return *this;
		}
		slice_iterator& operator++(int) {
			slice_iterator tmp = *this;
			p_ += step_;
			return tmp;
		}

		slice_iterator& operator--() {
			p_ -= step_;
			return *this;
		}
		slice_iterator& operator--(int) {
			slice_iterator tmp = *this;
			p_ -= step_;
			return tmp;
		}

		pointer operator->() const {
			return p_.operator->();
		}
		reference operator*() const {
			return *p_;
		}

		slice_iterator& operator=(const slice_iterator& lhs) {
			p_ = lhs.p_;
			return *this;
		}

		friend difference_type operator-(const slice_iterator& lhs, const slice_iterator& rhs) {
			return (lhs.p_ - rhs.p_) / lhs.step_;
		}

		bool operator<(const slice_iterator& rhs) {
			return p_ < rhs.p_;
		}
		bool operator>(const slice_iterator& rhs) {
			return p_ > rhs.p_;
		}
		bool operator<=(const slice_iterator& rhs) {
			return p_ <= rhs.p_;
		}
		bool operator>=(const slice_iterator& rhs) {
			return p_ >= rhs.p_;
		}
		bool operator==(const slice_iterator& rhs) {
			return p_ == rhs.p_;
		}
		bool operator!=(const slice_iterator& rhs) {
			return p_ != rhs.p_;
		}

		iterator_t& backend() {
			return p_;
		}
		const iterator_t& backend() const {
			return p_;
		}

	private:
		iterator_t p_;
		const difference_type step_;
	};

	template< class array_t >
	static fp_t sum(const array_t& a) {
		fp_t s = 0;
		for(typename array_t::const_iterator i = a.begin(), end = a.end(); i != end; ++i)
			s += *i;
		return s;
	}

	// helper structure to get dx[i] regardless of whether it given by one number or by array of
	// numbers
	struct dim_subscript {
		dim_subscript(const fp_storage_array_t& dim)
			: dim_(dim), sum_(0)
		{
			if(dim_.size() == 1)
				ss_fcn_ = &dim_subscript::ss_const_dim;
			else
				ss_fcn_ = &dim_subscript::ss_array_dim;
		}

		fp_storage_type_t ss_const_dim(i_type_t idx) {
			return static_cast< fp_storage_type_t >(fp_type_t(dim_[0] * idx));
		}

		fp_storage_type_t ss_array_dim(i_type_t idx) {
			fp_type_t tmp = sum_;
			sum_ += dim_[idx];
			return static_cast< fp_storage_type_t>(tmp);
		}

		void reset() { sum_ = 0; }

		fp_type_t operator[](i_type_t idx) {
			return (this->*ss_fcn_)(idx);
		}

	private:
		const fp_storage_array_t& dim_;
		fp_storage_type_t (dim_subscript::*ss_fcn_)(i_type_t);
		fp_type_t sum_;
	};

	static spfp_storarr_t gen_coord(int_t nx, int_t ny, spfp_storarr_t dx, spfp_storarr_t dy) {
		using namespace std;
		typedef typename fp_storarr_t::value_type value_t;

		// create subscripters
		typename inner::dim_subscript dxs(*dx);
		typename inner::dim_subscript dys(*dy);

		// if dimension offset is given as array, then size should be taken from array size
		if(dx->size() > 1) nx = dx->size();
		if(dy->size() > 1) ny = dy->size();

		// create arrays
		spfp_storarr_t coord = BS_KERNEL.create_object(fp_storarr_t::bs_type());
		if(!coord) return NULL;

		// fill coord
		// coord is simple grid
		coord->init((nx + 1)*(ny + 1)*6, value_t(0));
		typename fp_storage_array_t::iterator pcd = coord->begin();
		for(i_type_t iy = 0; iy <= ny; ++iy) {
			fp_t cur_y = dys[iy];
			for(i_type_t ix = 0; ix <= nx; ++ix) {
				pcd[0] = pcd[3] = dxs[ix];
				pcd[1] = pcd[4] = cur_y;
				pcd[5] = 1; // pcd[2] = 0 from init
				pcd += 6;
			}
			dxs.reset();
		}

		return coord;
	}

	void init_minmax(mesh_grdecl& m) const {
		i_type_t i, n;
		fp_storage_type_t *it;

		// init ZCORN
		m.min_z = *(std::min_element(zcorn_->begin(), zcorn_->end()));
		m.max_z = *(std::max_element(zcorn_->begin(), zcorn_->end()));

		n = coord_->size();

		m.max_x = m.min_x = m.coord_array[0];
		m.max_y = m.min_y = m.coord_array[1];

		for (i = 0; i < n; i += 6) {
			it = m.coord_array + i;
			// move matching points apart
			if (it[2] == it[5])
				it[5] += 1.0f;

			//looking for max&min coordinates
			if (m.min_x > it[0]) m.min_x = it[0];
			if (m.min_x > it[3]) m.min_x = it[3];

			if (m.min_y > it[1]) m.min_y = it[1];
			if (m.min_y > it[4]) m.min_y = it[4];

			if (m.max_x < it[0]) m.max_x = it[0];
			if (m.max_x < it[3]) m.max_x = it[3];

			if (m.max_y < it[1]) m.max_y = it[1];
			if (m.max_y < it[4]) m.max_y = it[4];
		}
	}

	template< class array_t >
	static void resize_zcorn(array_t& zcorn, int_t nx, int_t ny, int_t new_nx, int_t new_ny) {
		typedef typename array_t::iterator v_iterator;
		typedef typename array_t::value_type value_t;
		typedef typename array_t::size_type size_t;

		// modify zcorn
		// reserve memory
		int_t nz = (zcorn.size() >> 3) / (nx  * ny);
		zcorn.reserve(new_nx * new_ny * nz * 8);
		const int_t delta = 4 * (new_nx * new_ny - nx * ny);
		// process planes
		value_t z;
		size_t offset;
		for(int_t i = 2*nz - 1; i >= 0; --i) {
			// select plane
			offset = static_cast< size_t >(i * (nx * ny * 4));
			// assume that all z-plane values are equal!
			z = zcorn[offset];
			for(int_t j = 0; j < delta; ++j)
				zcorn.insert(zcorn.begin() + offset, z);
		}
	}

	static void insert_column(int_t nx, int_t ny, fp_storvec_t& coord, fp_storvec_t& zcorn, fp_storage_type_t where) {
		using namespace std;

		typedef typename fp_storvec_t::iterator v_iterator;
		typedef slice_iterator< v_iterator, 6 > dim_iterator;

		// reserve mem for insterts
		coord.reserve((nx + 2)*(ny + 1)*6);

		// find a place to insert
		dim_iterator pos = lower_bound(dim_iterator(coord.begin()), dim_iterator(coord.begin()) + (nx + 1), where);
		//if(pos == dim_iterator(coord.begin()) + (nx + 1)) return;
		int_t ins_offset = pos.backend() - coord.begin();

		// process all rows
		fp_stor_t y, z1, z2;
		v_iterator vpos;
		for(int_t i = ny; i >= 0; --i) {
			// save y and z values
			vpos = coord.begin() + i*(nx + 1)*6 + ins_offset;
			if(ins_offset == (nx + 1)*6) {
				// insert at the boundary
				y = *(vpos - 5); z1 = *(vpos - 4); z2 = *(vpos - 1);
			}
			else {
				// insert in the beginning/middle of row
				y = *(vpos + 1); z1 = *(vpos + 2); z2 = *(vpos + 5);
			}
			// insert new vector
			insert_iterator< fp_storvec_t > ipos(coord, vpos);
			*ipos++ = where; *ipos++ = y; *ipos++ = z1;
			*ipos++ = where; *ipos++ = y; *ipos = z2;
		}

		// update zcorn
		resize_zcorn(zcorn, nx, ny, nx + 1, ny);
	}

	static void insert_row(int_t nx, int_t ny, fp_storvec_t& coord, fp_storvec_t& zcorn, fp_storage_type_t where) {
		using namespace std;
		typedef typename fp_storvec_t::iterator v_iterator;
		typedef slice_iterator< v_iterator > dim_iterator;
		const int_t ydim_step = 6 * (nx + 1);

		// reserve mem for insterts
		coord.reserve((nx + 1)*(ny + 2)*6);

		// find a place to insert
		const dim_iterator search_end = dim_iterator(coord.begin() + 1, ydim_step) + (ny + 1);
		dim_iterator pos = lower_bound(dim_iterator(coord.begin() + 1, ydim_step), search_end, where);
		//if(pos == search_end) return;
		v_iterator ins_point = pos.backend() - 1;

		// make cache of x values from first row
		spfp_storvec_t cache_x = BS_KERNEL.create_object(fp_storvec_t::bs_type());
		cache_x->resize(nx + 1);
		typedef slice_iterator< v_iterator, 3 > hdim_iterator;
		copy(hdim_iterator(coord.begin()), hdim_iterator(coord.begin() + (nx + 1)*6), cache_x->begin());

		// insert row
		insert_iterator< fp_storvec_t > ipos(coord, ins_point);
		v_iterator p_x = cache_x->begin();
		fp_stor_t z1 = *(coord.begin() + 2), z2 = *(coord.begin() + 5);
		for(int_t i = 0; i <= nx; ++i) {
			*ipos++ = *p_x++; *ipos++ = where; *ipos++ = z1;
			*ipos++ = *p_x++; *ipos++ = where; *ipos++ = z2;
		}

		// update zcorn
		resize_zcorn(zcorn, nx, ny, nx, ny + 1);
	}

	template< class array_t >
	static spfp_storarr_t coord2deltas(const array_t& src) {
		typedef typename array_t::const_iterator carr_iterator;

		smart_ptr< fp_storarr_t > res = BS_KERNEL.create_object(fp_storarr_t::bs_type());
		if(src.size() < 2) return res;
		res->resize(src.size() - 1);
		typename fp_storarr_t::iterator p_res = res->begin();
		carr_iterator a = src.begin(), b = a, end = src.end();
		for(++b; b != end; ++b)
			*p_res++ = *b - *a++;
		return res;
	}

	struct proc_ray {
		template< int_t direction, class = void >
		struct dir_ray {
			enum { dir = direction };

			template< class ray_t >
			static typename ray_t::iterator end(ray_t& ray) {
				return ray.end();
			}

			template< class ray_t >
			static typename ray_t::iterator last(ray_t& ray) {
				return --ray.end();
			}

			template< class ray_t >
			static typename ray_t::iterator closest_bound(ray_t& ray, fp_t v) {
				return std::upper_bound(ray.begin(), ray.end(), v);
			}

			static fp_t min(fp_t f, fp_t s) {
				return std::min(f, s);
			}
		};

		template< class unused >
		struct dir_ray< -1, unused > {
			enum { dir = -1 };

			template< class ray_t >
			static typename ray_t::iterator end(ray_t& ray) {
				return --ray.begin();
			}

			template< class ray_t >
			static typename ray_t::iterator last(ray_t& ray) {
				return ray.begin();
			}

			template< class ray_t >
			static typename ray_t::iterator closest_bound(ray_t& ray, fp_t v) {
				return std::upper_bound(ray.begin(), ray.end(), v)--;
			}

			static fp_t min(fp_t f, fp_t s) {
				return std::max(f, s);
			}
		};

		template< class ray_t, class predicate >
		static fp_t find_cell(ray_t& coord, predicate p) {
			typedef typename ray_t::iterator ray_iterator;
			if(coord.size() < 2) return 0;
			// find max cell size
			fp_t cell_sz = 0;
			ray_iterator a = coord.begin(), b = a, end = coord.end();
			for(++b; b != end; ++b) {
				cell_sz = p(*b - *a++, cell_sz);
				//if(tmp > max_cell) max_cell = tmp;
			}
			return cell_sz;
		}

		template< class ray_t >
		static fp_t find_max_cell(ray_t& coord) {
			return find_cell(coord, std::max< fp_t >);
		}

		template< class ray_t >
		static fp_t find_min_cell(ray_t& coord) {
			return find_cell(coord, std::min< fp_t >);
		}


		template< class ray_t, class dir_ray_t >
		static void go(ray_t& ray, fp_t start_point, fp_stor_t d, fp_stor_t a, dir_ray_t dr) {
			using namespace std;
			const int_t dir = static_cast< int_t >(dir_ray_t::dir);
			// find where new point fall to
			const fp_t max_sz = find_max_cell(ray);
			const fp_t max_front = *dr.last(ray);
			fp_t cell_sz = d;
			fp_t wave_front = dr.min(start_point + dir * 0.5 * d, max_front);

			// make refined ray
			// add refinement grid
			fp_set ref_ray;
			while(cell_sz <= max_sz && abs(wave_front - max_front) > 0) {
				ref_ray.insert(wave_front);
				cell_sz *= a;
				wave_front = dr.min(wave_front + dir * cell_sz, max_front);
			}

			// merge refinement with original grid
			copy(ray.begin(), ray.end(), insert_iterator< fp_set >(ref_ray, ref_ray.begin()));
			// copy results back
			ray.clear();
			copy(ref_ray.begin(), ref_ray.end(), insert_iterator< ray_t >(ray, ray.begin()));
		}

		template< class ray_t >
		static int_t kill_tight_cells(ray_t& ray, fp_t min_cell_sz) {
			typedef typename ray_t::iterator ray_iterator;

			if(ray.size() < 2) return 0;
			int_t merge_cnt = 0;
			ray_iterator a = ray.begin(), b = a, end = ray.end();
			for(++b; b != end; ++b) {
				if(*b - *a < min_cell_sz) {
					ray.erase(a++);
					++merge_cnt;
				}
				else ++a;
			}
			return merge_cnt;
		}
	};

	template< class ray_t >
	static void refine_mesh_impl(ray_t& coord, fp_stor_t point, fp_stor_t d, fp_stor_t a) {
		using namespace std;
		typedef typename fp_storvec_t::iterator v_iterator;
		typedef typename fp_storvec_t::const_iterator cv_iterator;

		// sanity check
		if(!coord.size()) return;

		// process coord in both directions
		proc_ray::go(coord, point, d, a, typename proc_ray::template dir_ray< 1 >());
		proc_ray::go(coord, point, d, a, typename proc_ray::template dir_ray< -1 >());
	}

	static coord_zcorn_pair refine_mesh(int_t& nx, int_t& ny, sp_fp_storage_array_t coord, sp_fp_storage_array_t zcorn, sp_fp_storage_array_t points) {
		using namespace std;

		typedef typename fp_storvec_t::iterator v_iterator;
		typedef slice_iterator< v_iterator, 6 > dim_iterator;

		// sanity check
		if(!coord || !zcorn || !points) return coord_zcorn_pair();

		// convert coord & zcorn to shared vectors
		//spfp_storvec_t vcoord = BS_KERNEL.create_object(fp_storvec_t::bs_type());
		//if(vcoord) vcoord->init_inplace(coord->get_container());
		//else return coord_zcorn_pair();

		//spfp_storvec_t vzcorn = BS_KERNEL.create_object(fp_storvec_t::bs_type());
		//if(vzcorn) vzcorn->init_inplace(zcorn->get_container());
		//else return coord_zcorn_pair();

		vector< fp_stor_t > vzcorn(zcorn->begin(), zcorn->end());

		// build x and y coord maps
		//spfp_storvec_t x = BS_KERNEL.create_object(fp_storvec_t::bs_type());
		fp_set x;
		copy(dim_iterator(coord->begin()), dim_iterator(coord->begin()) + (nx + 1),
				insert_iterator< fp_set >(x, x.begin()));

		//spfp_storvec_t y = BS_KERNEL.create_object(fp_storvec_t::bs_type());
		//y->resize(ny + 1);
		fp_set y;
		const int_t ydim_step = 6 * (nx + 1);
		dim_iterator a(coord->begin() + 1, ydim_step), b = a;
		b += ny + 1;
		for(; a != b; ++a)
			y.insert(*a);
		//copy(dim_iterator(coord->begin() + 1, ydim_step), dim_iterator(coord->begin() + 1, ydim_step) + (ny + 1),
		//		insert_iterator< fp_set >(y, y.begin()));

		typename fp_storage_array_t::const_iterator pp = points->begin(), p_end = points->end();
		// make (p_end - p_begin) % 4 = 0
		p_end -= (p_end - pp) % 6;

		// process points in turn
		// points array: {(x, y, dx, dy, ax, ay)}
		fp_stor_t x_coord, y_coord, dx, dy, ax, ay;
		fp_stor_t min_dx, min_dy;
		bool first_point = true;
		while(pp != p_end) {
			x_coord = *(pp++); y_coord = *(pp++);
			dx = *(pp++); dy = *(pp++);
			ax = *(pp++); ay = *(pp++);
			if(first_point) {
				min_dx = dx; min_dy = dy;
				first_point = false;
			}
			else {
				min_dx = min(min_dx, dx);
				min_dy = min(min_dx, dy);
			}
			if(dx != 0)
				refine_mesh_impl(x, x_coord, dx, ax);
			if(dy != 0)
				refine_mesh_impl(y, y_coord, dy, ay);
		}

		// kill too tight cells in x & y directions
		while(proc_ray::kill_tight_cells(x, BOUND_MERGE_THRESHOLD * min_dx)) {}
		while(proc_ray::kill_tight_cells(y, BOUND_MERGE_THRESHOLD * min_dy)) {}

		// update zcorn
		resize_zcorn(vzcorn, nx, ny, x.size() - 1, y.size() - 1);
		// create bs_array from new zcorn
		spfp_storarr_t rzcorn = BS_KERNEL.create_object(fp_storarr_t::bs_type());
		if(!rzcorn) return coord_zcorn_pair();
		rzcorn->resize(vzcorn.size());
		copy(vzcorn.begin(), vzcorn.end(), rzcorn->begin());

		// DEBUG
		spfp_storarr_t delta_x = coord2deltas(x);
		spfp_storarr_t delta_y = coord2deltas(y);
		// check sum
		//fp_t sum_x = sum(*delta_x);
		//fp_t sum_y = sum(*delta_y);
		nx = x.size() - 1;
		ny = y.size() - 1;

		// rebuild grid based on processed x_coord & y_coord
		return coord_zcorn_pair(gen_coord(nx, ny,
				//coord2deltas(x), coord2deltas(y)),
				delta_x, delta_y),
				rzcorn);
	}

	// hold reference to coord and czron arrays if generated internally
	sp_fp_storage_array_t coord_;
	sp_fp_storage_array_t zcorn_;
};

template<class strategy_t>
mesh_grdecl<strategy_t>::mesh_grdecl ()
	: pinner_(new inner), coord_array(0), zcorn_array(0)
{}

template< class strategy_t >
std::pair< typename mesh_grdecl< strategy_t >::sp_fp_storage_array_t, typename mesh_grdecl< strategy_t >::sp_fp_storage_array_t >
mesh_grdecl< strategy_t >::gen_coord_zcorn(i_type_t nx, i_type_t ny, i_type_t nz, sp_fp_storage_array_t dx, sp_fp_storage_array_t dy, sp_fp_storage_array_t dz) {
	using namespace std;
	typedef std::pair< sp_fp_storage_array_t, sp_fp_storage_array_t > ret_t;
	typedef typename fp_storage_array_t::value_type value_t;

	// create subscripter
	if(!dx || !dy || !dz) return ret_t(NULL, NULL);
	if(!dx->size() || !dy->size() || !dz->size()) return ret_t(NULL, NULL);

	// if dimension offset is given as array, then size should be taken from array size
	if(dz->size() > 1) nz = dz->size();

	// create zcorn array
	sp_fp_storage_array_t zcorn = BS_KERNEL.create_object(fp_storage_array_t::bs_type());
	if(!zcorn) return ret_t(NULL, NULL);

	// fill zcorn
	// very simple case
	typename inner::dim_subscript dzs(*dz);
	zcorn->init(nx*ny*nz*8);

	typename fp_storage_array_t::iterator pcd = zcorn->begin();
	const i_type_t plane_size = nx * ny * 4;
	fp_storage_type_t z_cache = dzs[0];
	for(i_type_t iz = 1; iz <= nz; ++iz) {
		pcd = fill_n(pcd, plane_size, z_cache);
		z_cache = dzs[iz];
		pcd = fill_n(pcd, plane_size, z_cache);
	}

	return ret_t(inner::gen_coord(nx, ny, dx, dy), zcorn);
}

template< class strategy_t >
std::pair< typename mesh_grdecl< strategy_t >::sp_fp_storage_array_t, typename mesh_grdecl< strategy_t >::sp_fp_storage_array_t >
mesh_grdecl< strategy_t >::refine_mesh(i_type_t& nx, i_type_t& ny, sp_fp_storage_array_t coord, sp_fp_storage_array_t zcorn, sp_fp_storage_array_t points) {
	return inner::refine_mesh(nx, ny, coord, zcorn, points);
}

template< class strategy_t >
void mesh_grdecl< strategy_t >::init_props(i_type_t nx, i_type_t ny, i_type_t nz, sp_fp_storage_array_t dx, sp_fp_storage_array_t dy, sp_fp_storage_array_t dz) {
	// generate COORD & ZCORN
	std::pair< sp_fp_storage_array_t, sp_fp_storage_array_t > cz = gen_coord_zcorn(nx, ny, nz, dx, dy, dz);
	if(cz.first && cz.first->size()) {
		pinner_->coord_ = cz.first;
		coord_array = &pinner_->coord_->ss(0);
	}
	if(cz.second && cz.second->size()) {
		pinner_->zcorn_ = cz.second;
		zcorn_array = &pinner_->zcorn_->ss(0);
	}

	// postinit
	pinner_->init_minmax(*this);
}

template<class strategy_t>
void mesh_grdecl<strategy_t>::init_props(const sp_idata_t &idata)
{
  base_t::init_props (idata);
  sp_fp_storage_array_t data_array;
  
  // init ZCORN
  data_array = idata->get_fp_non_empty_array("ZCORN");
  if (data_array->size()) {
	  pinner_->zcorn_ = data_array;
	  zcorn_array = &(*data_array)[0];
  }
  // init COORD
  data_array = idata->get_fp_non_empty_array("COORD");
  if (data_array->size()) {
	  pinner_->coord_ = data_array;
	  coord_array = &(*data_array)[0];
  }

  // postinit
  pinner_->init_minmax(*this);
}

template<class strategy_t>
void mesh_grdecl<strategy_t>::check_data() const
{
  base_t::check_data ();

  if (min_x < 0)
    bs_throw_exception (boost::format ("min_x = %d is out of range")% min_x);
  if (min_y < 0)
    bs_throw_exception (boost::format ("min_y = %d is out of range")% min_y);
  if (min_z < 0)
    bs_throw_exception (boost::format ("min_z = %d is out of range")% min_z);

  if (!coord_array)
    bs_throw_exception ("COORD array is not initialized");
  if (!zcorn_array)
    bs_throw_exception ("ZCORN array is not initialized");
}


template<class strategy_t>
inline void
mesh_grdecl<strategy_t>::calc_corner_point(const fp_storage_type_t z, const fp_storage_type_t *coord, fpoint3d_t &p)const
  {
    p.z = z;
    /*
    float temp = (p.z-m_Coord.pStart.z)/(m_Coord.pEnd.z-m_Coord.pStart.z);
    p.x = temp *(m_Coord.pEnd.x-m_Coord.pStart.x)+m_Coord.pStart.x;
    p.y = temp *(m_Coord.pEnd.y-m_Coord.pStart.y)+m_Coord.pStart.y;
    */
    float temp = (p.z - coord[2]) / (coord[5] - coord[2]);
    p.x = temp *(coord[3] - coord[0]) + coord[0];
    p.y = temp *(coord[4] - coord[1]) + coord[1];
  }
  
//! get element corners index in zcorn_array of block[i,j,k]
 /* nodes layout
     *                             X
     *                    0+-------+1
     *                    /|     / |
     *                  /  |   /   |
     *               2/-------+3   |
     *              Y |   4+--|----+5
     *                |   /Z  |   /
     *                | /     | /
     *              6 /-------/7
     */

template<class strategy_t>
inline void
mesh_grdecl<strategy_t>::get_element_zcorn_index (const i_type_t i, const i_type_t j, const i_type_t k, element_zcorn_i_type_t& element)  const
{
  //typename mesh_grdecl<strategy_t>::element_zcorn_i_type_t element;
  
  i_type_t index1 = i * 2 + j * 4 * nx + k * 8 * nx * ny;
  i_type_t index2 = index1 + 4 * nx * ny;

  element[0] = index1;
  element[1] = index1 + 1;
  element[2] = index1 + 2 * nx;
  element[3] = index1 + 2 * nx + 1;

  element[4] = index2;
  element[5] = index2 + 1;
  element[6] = index2 + 2 * nx;
  element[7] = index2 + 2 * nx + 1;
}

//! get element corners index in zcorn_array of block[i,j,k]
template<class strategy_t>
typename mesh_grdecl<strategy_t>::plane_zcorn_i_type_t
mesh_grdecl<strategy_t>::get_plane_zcorn_index (element_zcorn_i_type_t &element, 
                                                         element_plane_orientation_t orientation)  const
{
  typename mesh_grdecl<strategy_t>::plane_zcorn_i_type_t plane;
  switch (orientation)
    {
      case x_axis_minus:  //left_cross
        plane[0] = element[0];
        plane[1] = element[2];
        plane[2] = element[4];
        plane[3] = element[6];
        break;
      case x_axis_plus:   //right_cross
        plane[0] = element[1];
        plane[1] = element[3];
        plane[2] = element[5];
        plane[3] = element[7];
        break;
      case y_axis_minus:  //top_cross
        plane[0] = element[0];
        plane[1] = element[1];
        plane[2] = element[4];
        plane[3] = element[5];
        break;
      case y_axis_plus:   //bottom_cross
        plane[0] = element[2];
        plane[1] = element[3];
        plane[2] = element[6];
        plane[3] = element[7];
        break;
      case z_axis_minus:  //lower_cross
        plane[0] = element[0];
        plane[1] = element[2];
        plane[2] = element[1];
        plane[3] = element[3];
        break;
      case z_axis_plus:   //upper_cross
        plane[0] = element[4];
        plane[1] = element[6];
        plane[2] = element[5];
        plane[3] = element[7];
        break;
      
      default:
        bs_throw_exception ("Invalid orientation!");;
    }
  return plane;  
}

template<class strategy_t>
typename mesh_grdecl<strategy_t>::element_t
mesh_grdecl<strategy_t>::calc_element (const i_type_t index) const
  {
    i_type_t i, j, k;
    element_t element;
    
    inside_to_XYZ (index, i, j, k);
    calc_element (i, j, k, element);
    return element;
  }

template<class strategy_t>
void
mesh_grdecl<strategy_t>::calc_element (const i_type_t i, const i_type_t j, const i_type_t k, element_t &element) const
  {
    corners_t corners;

#ifdef _DEBUG
    BS_ASSERT ( (i >= 0) && (i < nx) && (j >= 0) && (j < ny) && (k >= 0) && (k < nz));
#endif

    /* nodes layout
     *                             X
     *                    0+-------+1
     *                    /|     / |
     *                  /  |   /   |
     *               2/-------+3   |
     *              Y |   4+--|----+5
     *                |   /Z  |   /
     *                | /     | /
     *              6 /-------/7
     */

    //define index

    i_type_t index1 = i * 2 + j * 4 * nx + k * 8 * nx * ny;//upper side
    i_type_t index2 = index1 + 4 * nx * ny;//lower side
    i_type_t iCOORD = i + j * (nx + 1);

    calc_corner_point (zcorn_array[index1], &coord_array[iCOORD * 6], corners[0]);
    calc_corner_point (zcorn_array[index1 + 1], &coord_array[(iCOORD + 1) * 6], corners[1]);
    calc_corner_point (zcorn_array[index1 + 2 * nx], &coord_array[(iCOORD + (nx + 1)) * 6], corners[2]);
    calc_corner_point (zcorn_array[index1 + 2 * nx + 1], &coord_array[(iCOORD + (nx + 1) + 1) * 6], corners[3]);

    calc_corner_point (zcorn_array[index2], &coord_array[iCOORD * 6], corners[4]);
    calc_corner_point (zcorn_array[index2 + 1], &coord_array[(iCOORD + 1) * 6], corners[5]);
    calc_corner_point (zcorn_array[index2 + 2 * nx], &coord_array[(iCOORD + (nx + 1)) * 6], corners[6]);
    calc_corner_point (zcorn_array[index2 + 2 * nx + 1], &coord_array[(iCOORD + (nx + 1) + 1) * 6], corners[7]);
    
    element.init (corners);
  }
  
template<class strategy_t>
bool mesh_grdecl<strategy_t>::is_small(const i_type_t i, const i_type_t j, const i_type_t k, fp_type_t eps)  const
  {
    if (k >= nz)
      return false;

    fp_type_t dz1, dz2, dz3, dz4; //height for each coord
    //define index
    i_type_t index1 = i*2+j*4*nx+k*8*nx*ny;	//lower side
    i_type_t index2 = index1+4*nx*ny;			//upper side
    dz1 = zcorn_array[index2]-zcorn_array[index1];
    dz2 = zcorn_array[index2+1]-zcorn_array[index1+1];
    dz3 = zcorn_array[index2+2*nx]-zcorn_array[index1+2*nx];
    dz4 = zcorn_array[index2+2*nx+1]-zcorn_array[index1+2*nx+1];

    if (dz1 <= eps && dz2 <= eps && dz3 <= eps && dz4 <= eps)
      return true;
    return false;
  }

template<class strategy_t>
void
mesh_grdecl<strategy_t>::get_plane_crossing_projection_on_all_axis(const plane_t &plane1, const plane_t &plane2, fpoint3d_t &A)const
  {
    quadrangle_t quad1, quad2;

    //project on YoZ
    for (size_t i = 0; i < N_PLANE_CORNERS; i++)
      {
        quad1[i] = fpoint2d(plane1[i].y, plane1[i].z);
        quad2[i] = fpoint2d(plane2[i].y, plane2[i].z);
      }

    A.x = get_quadrangle_crossing_area (quad1, quad2, EPS_SQ);

    //project on XoZ
    for (size_t i = 0; i < N_PLANE_CORNERS; i++)
      {
        quad1[i] = fpoint2d(plane1[i].x, plane1[i].z);
        quad2[i] = fpoint2d(plane2[i].x, plane2[i].z);
      }
    A.y = get_quadrangle_crossing_area (quad1, quad2, EPS_SQ);

    if ((A.x + A.y )!= 0.0)
      {
        //project on XoY
        for (size_t i = 0; i < N_PLANE_CORNERS; i++)
          {
            quad1[i] = fpoint2d (plane1[i].x, plane1[i].y);
            quad2[i] = fpoint2d (plane2[i].x, plane2[i].y);
          }
        A.z = get_quadrangle_crossing_area (quad1, quad2, EPS_SQ);
     }
  }

template<class strategy_t>
typename mesh_grdecl<strategy_t>::fp_type_t 
mesh_grdecl<strategy_t>::get_center_zcorn(const element_zcorn_i_type_t &element)const
  {
    fp_type_t res = 0.0;
    size_t i, n = element.size();
    
    for (i = 0; i < n; i++)
      res += zcorn_array[element[i]];
      
    return res/n;
  }



template<class strategy_t>
int mesh_grdecl<strategy_t>::init_ext_to_int()
{
  i_type_t *ext_to_int_data, *int_to_ext_data;
  write_time_to_log init_time ("Mesh initialization", ""); 
  item_array_t volumes_temp(n_elements);

  //tools::save_seq_vector ("actnum.bs.txt").save actnum_array;
  
  /* int splicing_num = splicing(volumes_temp); */
  
  //check_adjacency (1);
  //tools::save_seq_vector ("active_blocks.bs.txt").save actnum_array;
  
  check_adjacency ();
  
  //make proxy array
  ext_to_int->resize(n_elements);
  ext_to_int->assign(0);
  ext_to_int_data = &(*ext_to_int)[0];
  size_t n_count = 0;

  i_type_t nn_active = 0, i_index; //number of non-active previous cells
  for (i_type_t i = 0; i < nz; ++i)
    {
      for (i_type_t j = 0; j < ny; ++j)
        for (i_type_t k = 0; k < nx; ++k, ++n_count)
          {
            i_index = k + (nx * j) + (i * nx * ny);
            
            if (!actnum_array[i_index])
              {
                nn_active++;
                ext_to_int_data[n_count] = -1;
              }
            else
              ext_to_int_data[n_count] = i_index-nn_active;
          }
    }

  //tools::save_seq_vector ("ext_to_int.bs.txt").save (ext_to_int);

  init_int_to_ext();
  int_to_ext_data = &(*int_to_ext)[0];
  
  //fill volume array (except non-active block and using proxy array)
  volumes->resize(n_active_elements);
  fp_type_t *volumes_data = &(*volumes)[0];
  
  for (int i = 0; i < n_active_elements; ++i)
    volumes_data[i] = volumes_temp[int_to_ext_data[i]];
    
  calc_depths();
  

  //bs_throw_exception ("NOT IMPL YET");
  return n_active_elements;
}

template<class strategy_t>
void mesh_grdecl<strategy_t>::splice_two_blocks (const i_type_t i, const i_type_t j, const i_type_t k, const i_type_t k1)
{
  i_type_t index, index1, index2;

  index1 = i * 2 + j * 4 * nx + k1 * 8 * nx * ny; //upper side of [i,j,k1]
  index2 = i * 2 + j * 4 * nx + (k1 * 8 + 4) * nx * ny; //lower side of [i,j,k1]
  if (k > k1)
    {
      index = i * 2 + j * 4 * nx + k * 8 * nx * ny; //upper side of [i,j,k]

      zcorn_array[index] = zcorn_array[index1];
      zcorn_array[index + 1] = zcorn_array[index1 + 1];
      zcorn_array[index + 2 * nx] = zcorn_array[index1 + 2 * nx];
      zcorn_array[index + 2 * nx + 1] = zcorn_array[index1 + 2 * nx + 1];

      zcorn_array[index2] = zcorn_array[index1];
      zcorn_array[index2 + 1] = zcorn_array[index1 + 1];
      zcorn_array[index2 + 2 * nx] = zcorn_array[index1 + 2 * nx];
      zcorn_array[index2 + 2 * nx + 1] = zcorn_array[index1 + 2 * nx + 1];
    }
  else
    {
      index = i * 2 + j * 4 * nx + (k * 8 + 4) * nx * ny; //lower side of [i,j,k]

      zcorn_array[index] = zcorn_array[index2];
      zcorn_array[index + 1] = zcorn_array[index2 + 1];
      zcorn_array[index + 2 * nx] = zcorn_array[index2 + 2 * nx];
      zcorn_array[index + 2 * nx + 1] = zcorn_array[index2 + 2 * nx + 1];

      zcorn_array[index1] = zcorn_array[index2];
      zcorn_array[index1 + 1] = zcorn_array[index2 + 1];
      zcorn_array[index1 + 2 * nx] = zcorn_array[index2 + 2 * nx];
      zcorn_array[index1 + 2 * nx + 1] = zcorn_array[index2 + 2 * nx + 1];
    }
}

template<class strategy_t>
bool mesh_grdecl<strategy_t>::are_two_blocks_close (const i_type_t i, const i_type_t j, const i_type_t k, const i_type_t k1)
{
  i_type_t index, index1;
  if (k > k1)
    {
      index = i * 2 + j * 4 * nx + k * 8 *nx * ny; //upper side of [i,j,k]
      index1 = i * 2 + j * 4 * nx + (k1 * 8 + 4) * nx * ny; //lower side of [i,j,k1]
    }
  else
    {
      index = i * 2 + j * 4 * nx + (k * 8 + 4) * nx * ny; //lower side of [i,j,k]
      index1 = i * 2 + j * 4 * nx + k1 * 8 * nx * ny; //upper side of [i,j,k1]
    }

  if ((fabs (zcorn_array[index1] - zcorn_array[index]) < max_thickness) &&
      (fabs (zcorn_array[index1 + 1] - zcorn_array[index + 1]) < max_thickness) &&
      (fabs (zcorn_array[index1 + 2 * nx] - zcorn_array[index + 2 * nx]) < max_thickness) &&
      (fabs (zcorn_array[index1 + 2 * nx + 1] - zcorn_array[index + 2 * nx + 1]) < max_thickness))
    {
      return true;
    }
  else
    {
      return false;
    }
}

template<class strategy_t>
bool mesh_grdecl<strategy_t>::check_adjacency(int shift_zcorn)
{
  i_type_t i, j, k;
  i_type_t index, zindex, zindex1;
  i_type_t n_adjacent = 0;
   
  for (i = 0; i < nx; ++i)
    for (j = 0; j < ny; ++j)
      for (k = 0; k < nz; ++k)
        {
          
          index = i + j * nx + k * nx * ny;
          
          
          // miss inactive blocks
          if (!actnum_array[index])
            {
              continue;
            }
                       
          // check blocks adjacency
          zindex = i * 2 + j * 4 * nx + k * 8 *nx * ny;
          zindex1 = zindex + 2; // check next by x
         
          if (i + 1 == nx ||
              (zcorn_array[zindex + 1] == zcorn_array[zindex1] &&
              zcorn_array[zindex +  2 * nx + 1] == zcorn_array[zindex1 +  2 * nx] &&
              zcorn_array[zindex + 4 * nx * ny + 1] == zcorn_array[zindex1 + 4 * nx * ny ] &&
              zcorn_array[zindex + 4 * nx * ny + 2 * nx + 1] == zcorn_array[zindex1 + 4 * nx * ny + 2 * nx]))
            {
              
              zindex1 = zindex + 4 * nx; // check next by y
              if (j + 1 == ny ||
                  (zcorn_array[zindex + 2 * nx] == zcorn_array[zindex1] &&
                  zcorn_array[zindex + 2 * nx + 1] == zcorn_array[zindex1 +  1] &&
                  zcorn_array[zindex + 4 * nx * ny + 2 * nx] == zcorn_array[zindex1 + 4 * nx * ny] &&
                  zcorn_array[zindex + 4 * nx * ny + 2 * nx + 1] == zcorn_array[zindex1+ 4 * nx * ny + 1]))
                  {
                    n_adjacent++;
                  }
            }
            if (shift_zcorn && (i + j) % 2 == 1)
              {
                zcorn_array[zindex] += 2;
                zcorn_array[zindex + 1] += 2;
                zcorn_array[zindex +  2 * nx] += 2;
                zcorn_array[zindex +  2 * nx + 1] += 2;
                zcorn_array[zindex + 4 * nx * ny] += 2;
                zcorn_array[zindex + 4 * nx * ny + 1] += 2;
                zcorn_array[zindex + 4 * nx * ny +  2 * nx] += 2;
                zcorn_array[zindex + 4 * nx * ny +  2 * nx + 1] += 2;
              }
       }
  
  BOSOUT (section::mesh, level::medium) << "  adjacent cells:"<< n_adjacent <<" ("<< n_adjacent * 100 / (n_active_elements)<<"% active)" << bs_end;
  return (n_adjacent == n_active_elements);
}

template<class strategy_t>
int mesh_grdecl<strategy_t>::splicing(item_array_t& volumes_temp)
{
  i_type_t nCount = 0;
  fp_type_t vol_sum, vol_block;
  i_type_t i, j, k, k1;
  i_type_t small_block_top, big_block_top;
  i_type_t n_inactive_orig, n_inactive_vol, n_incative_splice;
  i_type_t index;
  element_t element;

#ifdef _DEBUG
    BS_ASSERT (zcorn_array != 0 && coord_array != 0);
#endif


  n_inactive_orig = 0;
  n_inactive_vol = 0;
  n_incative_splice = 0;

  for (i = 0; i < nx; ++i)
    for (j = 0; j < ny; ++j)
      {
        small_block_top = -1;
        big_block_top = -1;
        vol_sum = 0.0;
        for (k = 0; k < nz; ++k)
          {
            index = i + j * nx + k * nx * ny;
            
            // miss inactive blocks
            if (!actnum_array[index])
              {
                // blocks can`t be spliced across inactive blocks
                big_block_top = -1;
                small_block_top = -1;
                vol_sum = 0.0;
                n_inactive_orig++;
                continue;
              }
            calc_element (i, j, k, element);
            vol_block = element.calc_volume ();
            fp_type_t vol_block_poro = vol_block * poro_array[index];

            if (vol_block_poro <= minpv)
              {
                // block is too small, set as inactive
                actnum_array[index] = 0;
                ++nCount;
                n_inactive_vol++;
                // blocks can`t be spliced across inactive blocks
                vol_sum = 0.0;
                big_block_top = -1;
                small_block_top = -1;
                continue;
              }
            else if (vol_block_poro < minsv)
              {
                // this is small block

                volumes_temp[index] = vol_block;

                if (big_block_top != -1)
                  {
                    // this block is absorbed by bigger block above
                    splice_two_blocks (i, j, big_block_top, k);
                    // add volume this small block to the volume of big block
                    volumes_temp[i + j * nx + big_block_top * nx * ny] += vol_block;

                    // make block inactive
                    actnum_array[index] = 0;
                    ++nCount;
                    n_incative_splice++;
                    small_block_top = -1;
                  }

                // check if this block is close enough to next block
                if ((k < (nz - 1)) && (are_two_blocks_close (i, j, k, k + 1)))
                  {
                    if (big_block_top == -1)
                      {
                        if (small_block_top == -1)
                          {
                            // if already not spliced, can be spliced with lower block
                            small_block_top = k;
                          }

                        // aggregate volume of small blocks in case they`ll be spliced with lower block
                        vol_sum += vol_block;
                      }
                  }
                else
                  {
                    // this block and next block are not close, can`t be coupled
                    small_block_top = -1;
                    big_block_top = -1;
                    vol_sum = 0.0;
                    // TODO: make multperm = 0
                  }

              }
            else
              {
                // this is big block

                volumes_temp[index] = vol_block;

                if (small_block_top != -1)
                  {
                    // this block absorbes all smaller blocks above
                    for (k1 = k - 1; k1 >= small_block_top; --k1)
                      {
                        splice_two_blocks (i, j, k, k1);
                        n_incative_splice++;
                        // make small block inactive
                        actnum_array[i + j * nx + k1 * nx * ny] = 0;
                        ++nCount;
                      }

                    // add volume all small blocks above to the volume of this big block
                    volumes_temp[index] += vol_sum;
                    vol_sum = 0.0;
                    small_block_top = -1;
                  }

                // check if this block is close enough to next block
                if ((k < (nz - 1)) && (are_two_blocks_close (i, j, k, k + 1)))
                  {
                    // this block can absorb lower small blocks
                    big_block_top = k;
                  }
                else
                  {
                    // this block and next block are not close, can`t be coupled
                    small_block_top = -1;
                    big_block_top = -1;
                    // TODO: make multperm = 0
                  }
              }
           }
        }
  
  
  i_type_t n_total = n_active_elements + n_inactive_orig;
  /*
  if (n_total != nx * ny * nz)
  
    {
      BOSOUT (section::mesh, level::error) << "MESH_GRDECL total cells assert failed!" << bs_end;
      return -1;
    }  
  */      
  
  BOSOUT (section::mesh, level::medium) << "Mesh cells info:" << bs_end;
  BOSOUT (section::mesh, level::medium) << "  total: "<< n_total << bs_end; 
  BOSOUT (section::mesh, level::medium) << "  initial active: "<< n_active_elements <<" ("<< n_active_elements * 100 / (n_total)<<"%)" << bs_end;
  BOSOUT (section::mesh, level::medium) << "  marked inactive: "<< nCount << " (" << n_inactive_vol << " by volume, " << n_incative_splice << " by splice)" << bs_end;
  n_active_elements -= nCount;
  BOSOUT (section::mesh, level::medium) << "  total active: "<< n_active_elements <<" ("<< n_active_elements * 100 / (n_total)<<"%)" << bs_end;
  
  return nCount;
}


template<class strategy_t>
int mesh_grdecl<strategy_t >::calc_depths ()
{
  depths->resize(n_active_elements);
  fp_type_t *depths_data = &(*depths)[0];
  i_type_t *ext_to_int_data = &(*ext_to_int)[0];
  i_type_t index = 0;
  element_zcorn_i_type_t element;
  
  for (i_type_t k = 0; k < nz; ++k)
    for (i_type_t j = 0; j < ny; ++j)
      for (i_type_t i = 0; i < nx; ++i, ++index)
        {
          if (actnum_array[index])
            {
              get_element_zcorn_index(i, j, k, element);
              depths_data[ext_to_int_data[index]] = get_center_zcorn(element);
            }
        }
  return 0;
}


static int n_tran_calc = 0;

// calculating method have been taken from td eclipse (page 896)
// calc transmissibility between fully adjacent cells index1 and index2
template<class strategy_t>
typename mesh_grdecl<strategy_t>::fp_type_t 
mesh_grdecl<strategy_t>::calc_tran(const i_type_t ext_index1, const i_type_t ext_index2, const plane_t &plane1, 
                                       const fpoint3d_t &center1, const fpoint3d_t &center2, direction d_dir, plane_t* plane2) const
{
  fp_type_t tran;
  fpoint3d_t D1, D2, A, plane_contact_center;
  
  n_tran_calc ++;
  
  // if (plane2 != 0) then column is not adjacent - but there still can be adjacent cells, so check it
  if (plane2 == 0 || ((plane1[0].z == (*plane2)[0].z) && (plane1[1].z == (*plane2)[1].z) && (plane1[2].z == (*plane2)[2].z) && (plane1[3].z == (*plane2)[3].z)))
  //if (plane2 == 0)
    {
      get_plane_center(plane1, plane_contact_center);
      plane_contact_center.distance_to_point (center1, D1);
      plane_contact_center.distance_to_point (center2, D2);
      A = get_projection_on_all_axis_for_one_side(plane1);
    }
  else 
    {
      fpoint3d_t plane1_center, plane2_center;
      
      get_plane_center(plane1, plane1_center);
      get_plane_center(*plane2, plane2_center);
      
      plane1_center.distance_to_point (center1, D1);
      plane2_center.distance_to_point (center2, D2);
      
      // check simple cases of plane intersection  
      
      // 1. plane1 is completely inside plane2
      if ((plane1[0].z >= (*plane2)[0].z) && (plane1[1].z >= (*plane2)[1].z) && (plane1[2].z <= (*plane2)[2].z) && (plane1[3].z <= (*plane2)[3].z))
        A = get_projection_on_all_axis_for_one_side(plane1);
      else 
      // 2. plane2 is completely inside plane1  
      if ((plane1[0].z <= (*plane2)[0].z) && (plane1[1].z <= (*plane2)[1].z) && (plane1[2].z >= (*plane2)[2].z) && (plane1[3].z >= (*plane2)[3].z))
        A = get_projection_on_all_axis_for_one_side(*plane2);
      else
      // 3. plane1 is lower than plane2
      if ((plane1[0].z >= (*plane2)[0].z) && (plane1[1].z >= (*plane2)[1].z) && (plane1[0].z <= (*plane2)[2].z) && (plane1[1].z <= (*plane2)[3].z) && (plane1[2].z >= (*plane2)[2].z) && (plane1[3].z >= (*plane2)[3].z))
        {
          (*plane2)[0] = plane1[0];
          (*plane2)[1] = plane1[1];
          A = get_projection_on_all_axis_for_one_side(*plane2);
        }
      else    
       // 4. plane2 is lower than plane1
      if ((plane1[0].z <= (*plane2)[0].z) && (plane1[1].z <= (*plane2)[1].z) && (plane1[2].z >= (*plane2)[0].z) && (plane1[3].z >= (*plane2)[1].z) && (plane1[2].z <= (*plane2)[2].z) && (plane1[3].z <= (*plane2)[3].z))
        {
          (*plane2)[2] = plane1[2];
          (*plane2)[3] = plane1[3];
          A = get_projection_on_all_axis_for_one_side(*plane2);
        }
      else    
        get_plane_crossing_projection_on_all_axis(plane1, *plane2, A);
    }
  


  float koef1, koef2 ; //koef = (A,Di)/(Di,Di)
  
  koef1 = A*D1 / (D1*D1);
  koef2 = A*D2 / (D2*D2);
  
  #ifdef BS_MESH_WRITE_TRANSMISS_MATRIX   
        if (ext_index1 < 1000)
          {
             /*
            fprintf (fp, "[A: %lf; %lf; %lf K1: %lf, K2: %lf]\n", A.x, A.y, A.z, koef1, koef2);
           
            fprintf (fp, "[D1: %lf; %lf; %lf, D2: %lf; %lf; %lf, plane_center: %lf; %lf; %lf, center1: %lf; %lf; %lf, center2: %lf; %lf; %lf]", \
            D1.x, D1.y, D1.z, D2.x, D2.y, D2.z,  plane_contact_center.x, plane_contact_center.y, plane_contact_center.z,  
            center1.x, center1.y, center1.z, center2.x, center2.y, center2.z);
            fprintf (fp, "[D1: %lf; %lf; %lf, D2: %lf; %lf; %lf, plane_center: %lf; %lf; %lf, center1: %lf; %lf; %lf, center2: %lf; %lf; %lf]", \
            D1.x, D1.y, D1.z, D2.x, D2.y, D2.z,  plane_contact_center.x, plane_contact_center.y, plane_contact_center.z,  
            center1.x, center1.y, center1.z, center2.x, center2.y, center2.z);
            */
          }
  #endif //BS_MESH_WRITE_TRANSMISS_MATRIX   

  if (koef1 < 10e-16 || koef2 < 10e-16)
    {
      /*
      BOSWARN (section::mesh, level::warning) 
        << boost::format ("For indexes (%d, %d) transmissibility will be set to 0 because koef1 = 0 (%f) or koef2 = 0 (%f)")
        % ext_index1 % ext_index2 % koef1 % koef2 
        << bs_end;
      */
      return 0;
    }

  fp_type_t Ti, Tj;

  fp_type_t ntg_index1 = 1;
  fp_type_t ntg_index2 = 1;
  if (ntg_array)
    {
      ntg_index1 = ntg_array[ext_index1];
      ntg_index2 = ntg_array[ext_index2];
    }

  if (d_dir == along_dim1) //lengthwise OX
    {
      Ti = permx_array[ext_index1]*ntg_index1*koef1;
      Tj = permx_array[ext_index2]*ntg_index2*koef2;
      tran = darcy_constant / (1 / Ti + 1 / Tj);
      if (multx_array)
        tran *= multx_array[ext_index1];
    }
  else if (d_dir == along_dim2) //lengthwise OY
    {
      Ti = permy_array[ext_index1]*ntg_index1*koef1;
      Tj = permy_array[ext_index2]*ntg_index2*koef2;
      tran = darcy_constant / (1 / Ti + 1 / Tj);
      if (multy_array)
        tran *= multy_array[ext_index1];
    }
  else //lengthwise OZ
    {
      Ti = permz_array[ext_index1]*koef1;
      Tj = permz_array[ext_index2]*koef2;
      tran = darcy_constant / (1 / Ti + 1 / Tj);
      if (multz_array)
        tran *= multz_array[ext_index1];
    }
  
  /*
  #ifdef BS_MESH_WRITE_TRANSMISS_MATRIX   
    if (ext_index1 < 1000)
      fprintf (fp, "[Ti: %lf; Tj: %lf]", Ti, Tj);
  #endif //BS_MESH_WRITE_TRANSMISS_MATRIX  
  */
  return tran;
}



template<class strategy_t>
int mesh_grdecl<strategy_t>::build_jacobian_and_flux_connections (const sp_bcsr_t jacobian,const sp_flux_conn_iface_t flux_conn,
                                                                  sp_i_array_t boundary_array)

{
  return build_jacobian_and_flux_connections_add_boundary (jacobian, flux_conn, boundary_array);
}

template<class strategy_t>
void mesh_grdecl<strategy_t>::get_block_dx_dy_dz (i_type_t n_elem, fp_type_t &dx, fp_type_t &dy, fp_type_t &dz) const
  {
    element_t elem = calc_element(n_elem);
    point3d_t sizes = elem.get_dx_dy_dz (); 
    dx = sizes[0];
    dy = sizes[1];
    dz = sizes[2];
  }

template<class strategy_t>
float mesh_grdecl<strategy_t>:: get_block_dx(i_type_t n_elem) const
  {
    return calc_element(n_elem).get_dx();
  }

template<class strategy_t>
float mesh_grdecl<strategy_t>:: get_block_dy(i_type_t n_elem) const
  {
    return calc_element(n_elem).get_dy();
  }

template<class strategy_t>
float mesh_grdecl<strategy_t>:: get_block_dz(i_type_t n_elem) const
  {
    return calc_element(n_elem).get_dz();
  }

template<class strategy_t>
float  mesh_grdecl<strategy_t>::get_dtop(i_type_t n_elem) const
{
  element_t elem;
  
  elem = calc_element (n_elem);

  return elem.get_center().z - elem.get_dz();
}

template<class strategy_t>
boost::python::list mesh_grdecl<strategy_t>::calc_element_tops ()
{
  element_t element;
  sp_fp_array_t tops, prop;
  sp_i_array_t indexes;
  i_type_t i, j, k, c, ind, *indexes_data;
  fp_type_t *tops_data, *prop_data;
  boost::python::list myavi_list;

  tops = give_kernel::Instance().create_object(bs_array<fp_type_t>::bs_type());
  indexes = give_kernel::Instance().create_object(bs_array<i_type_t>::bs_type());
  prop = give_kernel::Instance().create_object(bs_array<fp_type_t>::bs_type());

  tops->resize (n_elements * 8 * 3);
  indexes->resize (n_elements * 8);
  prop->resize (n_elements);

  indexes_data = &(*indexes)[0];
  tops_data = &(*tops)[0];
  prop_data = &(*prop)[0];

  ind = 0;
   
  for (i = 0; i < nx; ++i)
	  for (j = 0; j < ny; ++j)
		  for (k = 0; k < nz; ++k, ++ind)
		    {
			
				/*
			  // check blocks adjacency
          zindex = i * 2 + j * 4 * nx + k * 8 *nx * ny;
          zindex1 = zindex + 2; // check next by x
         
          if (i + 1 == nx ||
              zcorn_array[zindex + 1] == zcorn_array[zindex1] &&
              zcorn_array[zindex +  2 * nx + 1] == zcorn_array[zindex1 +  2 * nx] &&
              zcorn_array[zindex + 4 * nx * ny + 1] == zcorn_array[zindex1 + 4 * nx * ny ] &&
              zcorn_array[zindex + 4 * nx * ny + 2 * nx + 1] == zcorn_array[zindex1 + 4 * nx * ny + 2 * nx])
            {
              
              zindex1 = zindex + 4 * nx; // check next by y
              if (j + 1 == ny ||
                  zcorn_array[zindex + 2 * nx] == zcorn_array[zindex1] &&
                  zcorn_array[zindex + 2 * nx + 1] == zcorn_array[zindex1 +  1] &&
                  zcorn_array[zindex + 4 * nx * ny + 2 * nx] == zcorn_array[zindex1 + 4 * nx * ny] &&
                  zcorn_array[zindex + 4 * nx * ny + 2 * nx + 1] == zcorn_array[zindex1+ 4 * nx * ny + 1])
                  {
                    n_adjacent++;
                  }
            }
	*/
		      calc_element (i, j, k, element);
			  prop_data[ind] = poro_array[ind];
			  for (c = 0; c < 8; ++c)
				{
				  tops_data[8 * 3 * ind + 3 * c] = element.get_corners()[c].x;
				  tops_data[8 * 3 * ind + 3 * c + 1] = element.get_corners()[c].y;
				  tops_data[8 * 3 * ind + 3 * c + 2] = element.get_corners()[c].z * 10;
				}

				
			  indexes_data[8 * ind] = 8 * ind;
			  indexes_data[8 * ind + 1] = 8 * ind + 1;
			  indexes_data[8 * ind + 2] = 8 * ind + 3;
			  indexes_data[8 * ind + 3] = 8 * ind + 2;

			  indexes_data[8 * ind + 4] = 8 * ind + 4;
			  indexes_data[8 * ind + 5] = 8 * ind + 5;
			  indexes_data[8 * ind + 6] = 8 * ind + 7;
			  indexes_data[8 * ind + 7] = 8 * ind + 6;

/*			  
			  // planes indexes
			  indexes_data[24 * ind] = 8 * ind;
			  indexes_data[24 * ind + 1] = 8 * ind + 1;
			  indexes_data[24 * ind + 2] = 8 * ind + 3;
			  indexes_data[24 * ind + 3] = 8 * ind + 2;

			  indexes_data[24 * ind + 4] = 8 * ind + 4;
			  indexes_data[24 * ind + 5] = 8 * ind + 5;
			  indexes_data[24 * ind + 6] = 8 * ind + 7;
			  indexes_data[24 * ind + 7] = 8 * ind + 6;

			  indexes_data[24 * ind + 8] = 8 * ind;
			  indexes_data[24 * ind + 9] = 8 * ind + 2;
			  indexes_data[24 * ind + 10] = 8 * ind + 6;
			  indexes_data[24 * ind + 11] = 8 * ind + 4;

			  indexes_data[24 * ind + 12] = 8 * ind + 1;
			  indexes_data[24 * ind + 13] = 8 * ind + 3;
			  indexes_data[24 * ind + 14] = 8 * ind + 7;
			  indexes_data[24 * ind + 15] = 8 * ind + 5;

			  indexes_data[24 * ind + 16] = 8 * ind ;
			  indexes_data[24 * ind + 17] = 8 * ind + 1;
			  indexes_data[24 * ind + 18] = 8 * ind + 5;
			  indexes_data[24 * ind + 19] = 8 * ind + 4;

			  indexes_data[24 * ind + 20] = 8 * ind + 2;
			  indexes_data[24 * ind + 21] = 8 * ind + 3;
			  indexes_data[24 * ind + 22] = 8 * ind + 7;
			  indexes_data[24 * ind + 23] = 8 * ind + 6;
*/
			}

  myavi_list.append(tops);
  myavi_list.append(indexes);
  myavi_list.append(prop);

  return myavi_list;
}

template<class strategy_t>
boost::python::list mesh_grdecl<strategy_t>::calc_element_center ()
{
  element_t element;
  sp_fp_array_t centers, prop;
  i_type_t i, j, k /*, c */, ind /*, *indexes_data */;
  fp_type_t *centers_data, *prop_data;
  boost::python::list myavi_list;

  centers = give_kernel::Instance().create_object(bs_array<fp_type_t>::bs_type());
  prop = give_kernel::Instance().create_object(bs_array<fp_type_t>::bs_type());

  centers->resize (n_elements * 3);
  prop->resize (n_elements);

  centers_data = &(*centers)[0];
  prop_data = &(*prop)[0];

  ind = 0;
   
  for (i = 0; i < nx; ++i)
	  for (j = 0; j < ny; ++j)
		  for (k = 0; k < nz; ++k, ++ind)
		    {
		      calc_element (i, j, k, element);
			  centers_data[3 * ind] = element.get_center().x;
			  centers_data[3 * ind + 1] = element.get_center().y;
			  centers_data[3 * ind + 2] = element.get_center().z;
			}

  myavi_list.append(centers);
  myavi_list.append(prop);

  return myavi_list;
}


template<class strategy_t>
void mesh_grdecl<strategy_t>::generate_array()
{
#if 0
  i_type_t n_size = n_elements;
  poro_array->clear();
  ntg_array->clear();

  permx_array->clear();
  permy_array->clear();
  permz_array->clear();

  multx_array->clear();
  multy_array->clear();
  multz_array->clear();


  for (i_type_t i =0; i < n_size; ++i)
    {
      poro_array->push_back(0.2f);
      ntg_array->push_back(0.4f);

      permx_array->push_back(10.0f);
      permy_array->push_back(10.0f);
      permz_array->push_back(10.0f);

      multx_array->push_back(1.0f);
      multy_array->push_back(1.0f);
      multz_array->push_back(1.0f);
    }
#endif
}


template <typename strategy_t, typename loop_t>
struct build_jacobian_rows_class
{
  typedef typename strategy_t::i_type_t          i_type_t;
  typedef typename strategy_t::fp_type_t           fp_type_t;
  //typedef typename strategy_t::index_array_t    index_array_t;
  //typedef typename strategy_t::item_array_t     item_array_t;

  typedef mesh_grdecl <strategy_t>                mesh_t;
  typedef typename mesh_t::plane_t                plane_t;
  typedef typename mesh_t::element_zcorn_i_type_t  element_zcorn_i_type_t;

  build_jacobian_rows_class (mesh_grdecl <strategy_t> *mesh, loop_t *loop, std::set <i_type_t, std::less <i_type_t> > &boundary_set, i_type_t *rows_ptr)
  : mesh (mesh)
  , loop (loop)
  , boundary_set (boundary_set)
  , rows_ptr (rows_ptr)
  , nx (mesh->nx)
  , ny (mesh->ny)
  , nz (mesh->nz)
  {
  }

  void
  prepare (i_type_t i, i_type_t j, i_type_t k)
  {
    ext_index  = i + j * nx + k * nx * ny;
  }

  // check if (i, j) column of cells is adjacent to neigbours
  // that is true, if every cell of a column is fully adjacent to X and Y neighbour
  
  bool
  check_column_adjacency (i_type_t i, i_type_t j)
  {
    bool flag = true;
    i_type_t k, zindex, zindex1, index;
         
    for (k = 0; k < nz; ++k)
      {
        index = i + j * nx + k * nx * ny;
         
        // miss inactive blocks
        if (!mesh->actnum_array[index])
          {
            continue;
          }
          
        // check blocks adjacency
        zindex = i * 2 + j * 4 * nx + k * 8 *nx * ny;
        zindex1 = zindex + 2; // check next by x
       
        if (i + 1 == nx ||
            (mesh->zcorn_array[zindex + 1] == mesh->zcorn_array[zindex1] &&
            mesh->zcorn_array[zindex +  2 * nx + 1] == mesh->zcorn_array[zindex1 +  2 * nx] &&
            mesh->zcorn_array[zindex + 4 * nx * ny + 1] == mesh->zcorn_array[zindex1 + 4 * nx * ny ] &&
            mesh->zcorn_array[zindex + 4 * nx * ny + 2 * nx + 1] == mesh->zcorn_array[zindex1 + 4 * nx * ny + 2 * nx]))
          {
            
            zindex1 = zindex + 4 * nx; // check next by y
            if (j + 1 == ny ||
                (mesh->zcorn_array[zindex + 2 * nx] == mesh->zcorn_array[zindex1] &&
                mesh->zcorn_array[zindex + 2 * nx + 1] == mesh->zcorn_array[zindex1 +  1] &&
                mesh->zcorn_array[zindex + 4 * nx * ny + 2 * nx] == mesh->zcorn_array[zindex1 + 4 * nx * ny] &&
                mesh->zcorn_array[zindex + 4 * nx * ny + 2 * nx + 1] == mesh->zcorn_array[zindex1+ 4 * nx * ny + 1]))
                {
                  // cell is adjacent;
                }
             else
              {
                flag = false;
                break;
              }
          }
         else
          {
            flag = false;
            break;
          }  
       }
    loop->is_column_adjacent[j * nx + i] = flag;
    return flag;
  }

  void
  change_by_x (i_type_t i, i_type_t j, i_type_t k, i_type_t ext_index2, bool is_adjacent)
  {
    rows_ptr[mesh->convert_ext_to_int (ext_index) + 1]++;
    rows_ptr[mesh->convert_ext_to_int (ext_index2) + 1]++;
  }

  void
  change_by_y (i_type_t i, i_type_t j, i_type_t k, i_type_t ext_index2, bool is_adjacent)
  {
    rows_ptr[mesh->convert_ext_to_int (ext_index) + 1]++;
    rows_ptr[mesh->convert_ext_to_int (ext_index2) + 1]++;
  }

  void
  change_by_z (i_type_t i, i_type_t j, i_type_t k, i_type_t ext_index2, bool is_adjacent)
  {
    rows_ptr[mesh->convert_ext_to_int (ext_index) + 1]++;
    rows_ptr[mesh->convert_ext_to_int (ext_index2) + 1]++;
  }

  void
  add_boundary (i_type_t external_cell_index)
  {
    boundary_set.insert (external_cell_index);
  }

  mesh_grdecl <strategy_t>                  *mesh;
  loop_t                                    *loop;
  std::set <i_type_t, std::less <i_type_t> >  &boundary_set;
  i_type_t                                   *rows_ptr;
  i_type_t                                   nx;
  i_type_t                                   ny;
  i_type_t                                   nz;
  i_type_t                                   ext_index;
};

template <typename T, typename L, typename BS, typename RP>
build_jacobian_rows_class <T, L>
build_jacobian_rows (mesh_grdecl <T> *mesh, L *l, BS &bs, RP *rp)
{
  return build_jacobian_rows_class <T, L> (mesh, l, bs, rp);
}

template <typename strategy_t, typename loop_t>
struct build_jacobian_cols_class
{
  typedef typename strategy_t::i_type_t          i_type_t;
  typedef typename strategy_t::fp_type_t         fp_type_t;
  typedef std::vector<i_type_t>                   index_array_t;
  typedef typename strategy_t::fp_storage_type_t fp_storage_type_t;

  typedef mesh_grdecl <strategy_t>                mesh_t;
  typedef typename mesh_t::element_t              element_t;
  typedef typename mesh_t::plane_t                plane_t;
  typedef typename mesh_t::element_zcorn_i_type_t  element_zcorn_i_type_t;

  build_jacobian_cols_class (mesh_t *mesh, loop_t *loop, i_type_t *rows_ptr, i_type_t *cols_ind,
    i_type_t *cols_ind_transmis, fp_storage_type_t *values_transmis,
    i_type_t *matrix_block_idx_minus, i_type_t *matrix_block_idx_plus)
  : mesh (mesh)
  , loop (loop)
  , rows_ptr (rows_ptr)
  , cols_ind (cols_ind)
  , cols_ind_transmis (cols_ind_transmis)
  , values_transmis (values_transmis)
  , matrix_block_idx_minus (matrix_block_idx_minus)
  , matrix_block_idx_plus (matrix_block_idx_plus)
  , nx (mesh->nx)
  , ny (mesh->ny)
  , nz (mesh->nz)
  {
    //curIndex.assign (rows_ptr->begin (), rows_ptr->end ());
    i_type_t i;
    i_type_t n = mesh->n_active_elements;
    
    
    rows_ptr_tmp.resize (n);
    for (i = 0; i < n; ++i)
      {
        // let it point to first non-diagonal column in each row
        rows_ptr_tmp[i] = rows_ptr[i] + 1;
        cols_ind[rows_ptr[i]]= i;
      }
  }
  
  void
  prepare (i_type_t i, i_type_t j, i_type_t k)
  {
    ext_index  = i + j * nx + k * nx * ny;
    int_index  = mesh->convert_ext_to_int (ext_index);

    mesh->calc_element(i, j, k, element);
    center1  = element.get_center();
  }
  
    
  void
  change_jac_and_flux_conn( const i_type_t ext_index1, const i_type_t ext_index2, fp_type_t tran)
  {
    i_type_t index1 = mesh->convert_ext_to_int (ext_index);
    i_type_t index2 = mesh->convert_ext_to_int (ext_index2);
    
    cols_ind[rows_ptr_tmp[index1]] = index2;
    cols_ind[rows_ptr_tmp[index2]] = index1;

    matrix_block_idx_minus[loop->con_num] = rows_ptr[index1];
    matrix_block_idx_minus[loop->con_num + 1] = rows_ptr_tmp[index1];
    matrix_block_idx_plus[loop->con_num + 1] = rows_ptr[index2];
    matrix_block_idx_plus[loop->con_num] = rows_ptr_tmp[index2];
    
    ++rows_ptr_tmp[index1];
    ++rows_ptr_tmp[index2];
    
    cols_ind_transmis[loop->con_num] = index1;
    cols_ind_transmis[loop->con_num + 1] = index2;
    
    values_transmis[loop->con_num] = tran;
    values_transmis[loop->con_num + 1] = -tran;
    loop->con_num += 2;
  }

  bool
  check_column_adjacency (i_type_t i, i_type_t j)
  {
    return loop->is_column_adjacent[i + j * nx];
  }
  
  void
  add_boundary (i_type_t)
  {
  }

  void
  change_by_x (i_type_t i, i_type_t j, i_type_t k, i_type_t ext_index2, bool is_adjacent)
  {
    fp_type_t tran;
    plane_t plane1;
    element_t element2;
    
    mesh->calc_element(i, j, k, element2);
    fpoint3d center2  = element2.get_center ();
    
    element.get_plane (x_axis_plus, plane1);
    
     
    if (is_adjacent)
      {
        tran = mesh->calc_tran (ext_index, ext_index2, plane1, center1, center2, along_dim1);
      }
    else
      {
        plane_t plane2;
        element2.get_plane (x_axis_minus, plane2);
        tran = mesh->calc_tran (ext_index, ext_index2, plane1, center1, center2, along_dim1, &plane2);
      }
    
    if (tran != 0)
      {
        change_jac_and_flux_conn (ext_index, ext_index2, tran);
      }  
   
    #ifdef BS_MESH_WRITE_TRANSMISS_MATRIX   
        if (ext_index < 1000)
          fprintf (fp, " %d [%d;%d;%d] (%lf)", ext_index2, i, j, k, tran);
    #endif //BS_MESH_WRITE_TRANSMISS_MATRIX 
  }

  void
  change_by_y (i_type_t i, i_type_t j, i_type_t k, i_type_t ext_index2, bool is_adjacent)
  {
    fp_type_t tran;
    
    plane_t plane1;
    element_t element2;
    
    mesh->calc_element(i, j, k, element2);
    fpoint3d center2  = element2.get_center ();
    element.get_plane (y_axis_plus, plane1);
     
    if (is_adjacent)
      {
        tran = mesh->calc_tran (ext_index, ext_index2, plane1, center1, center2, along_dim2);
      }
    else
      {
        plane_t plane2;
        element2.get_plane (y_axis_minus, plane2);
        tran = mesh->calc_tran (ext_index, ext_index2, plane1, center1, center2, along_dim2, &plane2);
      }
     
    if (tran != 0)
      {
        change_jac_and_flux_conn (ext_index, ext_index2, tran);
      }
    #ifdef BS_MESH_WRITE_TRANSMISS_MATRIX   
        if (ext_index < 1000)
          fprintf (fp, " %d [%d;%d;%d] (%lf)", ext_index2, i, j, k, tran);
    #endif //BS_MESH_WRITE_TRANSMISS_MATRIX 
  }

  void
  change_by_z (i_type_t i, i_type_t j, i_type_t k, i_type_t ext_index2, bool is_adjacent)
  {
    fp_type_t tran;
    
    plane_t plane1;
    element_t element2;
    
    mesh->calc_element(i, j, k, element2);
    fpoint3d center2  = element2.get_center ();
    element.get_plane (z_axis_plus, plane1);
    
    tran = mesh->calc_tran (ext_index, ext_index2, plane1, center1, center2, along_dim3);
         
    if (tran != 0)
      {
        change_jac_and_flux_conn (ext_index, ext_index2, tran);
      }
    #ifdef BS_MESH_WRITE_TRANSMISS_MATRIX   
        if (ext_index < 1000)
          fprintf (fp, " %d [%d;%d;%d] (%lf)", ext_index2, i, j, k, tran);
    #endif //BS_MESH_WRITE_TRANSMISS_MATRIX  
  }

  mesh_t              *mesh;
  loop_t              *loop;
  i_type_t             *rows_ptr;
  i_type_t             *cols_ind;
  i_type_t             *cols_ind_transmis;
  fp_storage_type_t   *values_transmis;
  i_type_t             *matrix_block_idx_minus;
  i_type_t             *matrix_block_idx_plus;

  i_type_t             nx;
  i_type_t             ny;
  i_type_t             nz;

  // points to current column position for each row
  // while adding new columns
  index_array_t       rows_ptr_tmp; 

  i_type_t             ext_index; // index1
  i_type_t             int_index; // index_ijk

  element_t           element;
  fpoint3d            center1;

};

template <typename M, typename L, typename RP, typename CI, typename CT, typename FC>
build_jacobian_cols_class <M, L>
build_jacobian_cols (mesh_grdecl <M> *m, L *l, RP *rp, CI *ci, CT &conn_trans, FC &flux_conn)
{
  return build_jacobian_cols_class <M, L> (m, l, rp, ci,
    &(*conn_trans->get_cols_ind ())[0], &(*conn_trans->get_values ())[0],
    &(*flux_conn->get_matrix_block_idx_minus ())[0], &(*flux_conn->get_matrix_block_idx_plus ())[0]);
}

template <typename strategy_t>
struct build_jacobian_and_flux : boost::noncopyable
{
  typedef typename strategy_t::i_type_t          i_type_t;
  typedef typename strategy_t::fp_type_t         fp_type_t;
  typedef typename strategy_t::fp_storage_type_t fp_storage_type_t;
  //typedef typename strategy_t::index_array_t    index_array_t;
  //typedef typename strategy_t::item_array_t     item_array_t;

  typedef mesh_grdecl <strategy_t>                mesh_t;
  typedef typename mesh_t::plane_t                plane_t;
  typedef typename mesh_t::element_zcorn_i_type_t  element_zcorn_i_type_t;

  build_jacobian_and_flux (mesh_grdecl <strategy_t> *mesh)
  : mesh (mesh)
  , nx (mesh->nx)
  , ny (mesh->ny)
  , nz (mesh->nz)
  , con_num (0)
  {
    is_column_adjacent.assign (nx * ny, true);
  }

  template <typename loop_body_t>
  void
  cell_loop (loop_body_t loop_body)
  {
    i_type_t ext_index1, ext_index2;
    i_type_t last_k_x, last_k_y, k_x, k_y;
    bool is_adjacent;
    i_type_t n_adj_elems = 0, n_non_adj_elems = 0;
    
    element_zcorn_i_type_t zcorn_index1, zcorn_index2; 
    fp_storage_type_t *zcorn_array = mesh->zcorn_array;

    for (i_type_t i = 0; i < nx; ++i)
      {
        for (i_type_t j = 0; j < ny; ++j)
          {
            is_adjacent = loop_body.check_column_adjacency (i, j);
            
            if (is_adjacent)
              {   
                
                // simple loop
                
                for (i_type_t k = 0; k < nz; ++k)
                  {
                    ext_index1  = i + j * nx + k * nx * ny;
                    
                    //skip non-active cells
                    if (!mesh->actnum_array[ext_index1])
                      continue;
                      
                    n_adj_elems++;
                    
                    #ifdef BS_MESH_WRITE_TRANSMISS_MATRIX   
                    if (ext_index1 < 1000)
                      fprintf (fp, "\n%d [%d;%d;%d]", ext_index1, i, j, k);
                    #endif //BS_MESH_WRITE_TRANSMISS_MATRIX  
                      
                    loop_body.prepare (i, j, k);
                    
                    if (i+1 < nx && mesh->actnum_array[ext_index1 + 1])
                      {
                        loop_body.change_by_x (i + 1, j, k, ext_index1 + 1, true);
                      }
                      
                    if (j+1 < ny && mesh->actnum_array[ext_index1 + nx])
                      {
                        loop_body.change_by_y (i, j + 1, k, ext_index1 + nx, true);
                      }
                     
                    if (k+1 < nz && mesh->actnum_array[ext_index1 + nx * ny])
                      {
                        loop_body.change_by_z (i, j, k + 1, ext_index1 + nx * ny, true);
                      }
                  }
              }
            else
              {
                // complicated loop
                
                last_k_x = 0;
                last_k_y = 0;
                
                 /*                             X
                 *                    0+-------+1
                 *                    /|     / |
                 *                  /  |   /   |
                 *               2/-------+3   |
                 *              Y |   4+--|----+5
                 *                |   /Z  |   /
                 *                | /     | /
                 *              6 /-------/7
                 */
                            
                for (i_type_t k = 0; k < nz; ++k)
                  {
                    ext_index1  = i + j * nx + k * nx * ny;
                    
                    //skip non-active cells
                    if (!mesh->actnum_array[ext_index1])
                      continue;
                    
                    n_non_adj_elems++;
                    
                    #ifdef BS_MESH_WRITE_TRANSMISS_MATRIX   
                    if (ext_index1 < 1000)
                      fprintf (fp, "\n%d [%d;%d;%d]", ext_index1, i, j, k);
                    #endif //BS_MESH_WRITE_TRANSMISS_MATRIX  
                    
                    mesh->get_element_zcorn_index (i, j, k, zcorn_index1);
                    
                    loop_body.prepare (i, j, k);
                    
                    // if X neighbour exists and current element`s X+ plane is not a line
					if (i + 1 < nx && ((zcorn_array[zcorn_index1[1]] != zcorn_array[zcorn_index1[5]]) || (zcorn_array[zcorn_index1[3]] != zcorn_array[zcorn_index1[7]])))
                      {
                        k_x = last_k_x - 1;
                        
                        // search first possible neighbour
                        do
                          {
                            k_x++;
                            mesh->get_element_zcorn_index (i + 1, j, k_x, zcorn_index2);
                          }
                        while ((k_x < nz) && ((zcorn_array[zcorn_index1[1]] >= zcorn_array[zcorn_index2[4]]) && (zcorn_array[zcorn_index1[3]] >= zcorn_array[zcorn_index2[6]])));
                        
                        // calc all neihbours
                        while ((k_x < nz) && ((zcorn_array[zcorn_index1[5]] > zcorn_array[zcorn_index2[0]]) || (zcorn_array[zcorn_index1[7]] > zcorn_array[zcorn_index2[2]])))
                          {
                            if ((zcorn_array[zcorn_index1[5]] >= zcorn_array[zcorn_index2[4]]) && (zcorn_array[zcorn_index1[7]] >= zcorn_array[zcorn_index2[6]]))
                              {
                                // this (i + 1, j, k_x) neigbour won`t touch next (i, j, k + 1) element
                                last_k_x = k_x + 1;
                              }
                              
                            ext_index2 = ext_index1 + 1 + (k_x - k) * nx * ny;
                            
                            // if neighbour active and it`s X- plane is not a line
							if (mesh->actnum_array[ext_index2] && ((zcorn_array[zcorn_index2[0]] != zcorn_array[zcorn_index2[4]]) || (zcorn_array[zcorn_index2[2]] != zcorn_array[zcorn_index2[6]])))
                              {
                                loop_body.change_by_x (i + 1, j, k_x, ext_index2, false);
                              }
                            k_x++;
                            mesh->get_element_zcorn_index (i + 1, j, k_x, zcorn_index2);
                          }
                      }
                      
                    // if Y neighbour exists and current element`s Y+ plane is not a line
					if (j + 1 < ny && ((zcorn_array[zcorn_index1[2]] != zcorn_array[zcorn_index1[6]]) || (zcorn_array[zcorn_index1[3]] != zcorn_array[zcorn_index1[7]])))
                      {
                        k_y = last_k_y - 1;
                        
                        // search first possible neighbour
                        do
                          {
                            k_y++;
                            mesh->get_element_zcorn_index (i, j + 1, k_y, zcorn_index2);
                          }
                        while ((k_y < nz) && ((zcorn_array[zcorn_index1[2]] >= zcorn_array[zcorn_index2[4]]) && (zcorn_array[zcorn_index1[3]] >= zcorn_array[zcorn_index2[5]])));
                        
                        // calc all neighbours
                        while ((k_y < nz) && ((zcorn_array[zcorn_index1[6]] > zcorn_array[zcorn_index2[0]]) || (zcorn_array[zcorn_index1[7]] > zcorn_array[zcorn_index2[1]])))
                          {
                            if ((zcorn_array[zcorn_index1[6]] >= zcorn_array[zcorn_index2[4]]) && (zcorn_array[zcorn_index1[7]] >= zcorn_array[zcorn_index2[5]]))
                              {
                                // this (i, j + 1, k_y) neigbour won`t touch next (i, j, k + 1) element
                                last_k_y = k_y + 1;
                              }
                              
                            ext_index2 = ext_index1 + ny + (k_y - k) * nx * ny;
                            
                            // if neighbour active and it`s Y- plane is not a line
							if (mesh->actnum_array[ext_index2] && ((zcorn_array[zcorn_index2[0]] != zcorn_array[zcorn_index2[4]]) || (zcorn_array[zcorn_index2[1]] != zcorn_array[zcorn_index2[5]])))
                              {
                                loop_body.change_by_y (i, j + 1, k_y, ext_index2, false);
                              }
                            k_y++;
                            mesh->get_element_zcorn_index (i, j + 1, k_y, zcorn_index2);
                          }
                      }
                      
                    if (k + 1 < nz && mesh->actnum_array[ext_index1 + nx * ny])
                      {
                        loop_body.change_by_z (i, j, k + 1, ext_index1 + nx * ny, true);
                      }    
                  }
              }
          }
      }
    BOSWARN (section::mesh, level::warning)<< boost::format ("MESH_GRDECL: elements adj %d, non-adj %d, total %d") \
            % n_adj_elems % n_non_adj_elems % (n_adj_elems + n_non_adj_elems) << bs_end;  
    BOSWARN (section::mesh, level::warning)<< boost::format ("MESH_GRDECL: number of tran calcs is %d") % n_tran_calc << bs_end;  
  }

  mesh_grdecl <strategy_t>    *mesh;
  i_type_t                     nx;
  i_type_t                     ny;
  i_type_t                     nz;
  shared_vector <bool>        is_column_adjacent;
  i_type_t                     con_num;
};


template<class strategy_t>
int mesh_grdecl<strategy_t>::build_jacobian_and_flux_connections_add_boundary (const sp_bcsr_t jacobian,
                                                                               const sp_flux_conn_iface_t flux_conn,
                                                                               sp_i_array_t boundary_array)
{
  write_time_to_log init_time ("Mesh transmissibility calculation", ""); 
  
  
  i_type_t* rows_ptr, *cols_ind;
  i_type_t i, n_non_zeros;
  sp_bcsr_t conn_trans;
  
  n_connections = 0;
  jacobian->get_cols_ind()->clear();
  jacobian->get_rows_ptr()->clear();
  jacobian->init_struct(n_active_elements, n_active_elements, n_active_elements);
  
  rows_ptr = &(*jacobian->get_rows_ptr())[0];
  rows_ptr[0] = 0;

  std::vector<bool> is_butting(nx*ny,false);

  std::set<i_type_t, std::less<i_type_t> > boundary_set;

  build_jacobian_and_flux <strategy_t> build_jacobian (this);
  
 #ifdef BS_MESH_WRITE_TRANSMISS_MATRIX     
  fp = fopen ("transmiss.out", "w");
 #endif //BS_MESH_WRITE_TRANSMISS_MATRIX   

  //first step - define and fill - rows_ptr (jacobian)
  build_jacobian.cell_loop (build_jacobian_rows (this, &build_jacobian, boundary_set, rows_ptr));

  //////jacobian//////////////////////
  //sum rows_ptr
  for (i = 0; i < n_active_elements; ++i)
    {
      rows_ptr[i + 1] += rows_ptr[i] + 1;
    }
    
  n_non_zeros = rows_ptr[n_active_elements];
  n_connections = (n_non_zeros - n_active_elements) / 2;
    
  //create cols_ind
  jacobian->get_cols_ind()->resize(n_non_zeros);
  cols_ind = &(*jacobian->get_cols_ind())[0];


  ////////transmis/////////////////////////

  conn_trans = flux_conn->get_conn_trans();
  conn_trans->init (n_connections, 2 * n_connections, 1, 2 * n_connections);

  i_type_t *rows_ptr_transmis = &(*conn_trans->get_rows_ptr())[0];
  
  flux_conn->get_matrix_block_idx_minus ()->resize(n_connections * 2);
  flux_conn->get_matrix_block_idx_plus ()->resize(n_connections * 2);
  
  //i_type_t *matrix_block_idx_minus = &(*flux_conn->get_matrix_block_idx_minus ())[0];
  //i_type_t *matrix_block_idx_plus = &(*flux_conn->get_matrix_block_idx_plus ())[0];

  if (!n_connections)
    {
      for (i = 0; i < n_non_zeros; ++i)
        cols_ind[i] = i;
      return 0;
    }

  for (i = 0; i < n_connections + 1; ++i)
    rows_ptr_transmis[i] = i * 2;

  //second step - fill and define cols_ind
  build_jacobian.con_num = 0;
  build_jacobian.cell_loop (build_jacobian_cols (this, &build_jacobian, rows_ptr, cols_ind, conn_trans, flux_conn));


  //boundary_array.assign(boundary_set.begin(), boundary_set.end());
  
  #ifdef BS_MESH_WRITE_TRANSMISS_MATRIX  
    fflush (fp);
    fclose (fp);
  #endif //BS_MESH_WRITE_TRANSMISS_MATRIX 

  //return (int) boundary_array->size();
  return 0;
}

#ifdef _HDF5_MY
template<class strategy_t>
int mesh_grdecl<strategy_t>::create_array_hdf5(const char *dataset_name, H5::H5File &file_hdf5, H5::DataSet **dataset)
{
  // creating dataset_coords
  hsize_t dims[] = {0};
  hsize_t dims_max[] = {H5S_UNLIMITED};
  H5::DataSpace dataspace_coords(1, dims, dims_max);
  // set the dataset to be chunked
  H5::DSetCreatPropList cparms;
  hsize_t chunk_dims[] ={1};
  cparms.setChunk(1, chunk_dims);
  *dataset = new H5::DataSet(file_hdf5.createDataSet(dataset_name, H5::PredType::NATIVE_FLOAT, dataspace_coords, cparms));
  return 0;
}

template<class strategy_t>
bool mesh_grdecl<strategy_t>::file_open_activs_hdf5(const char* file_name, int is, int js, int ks, int it, int jt, int kt)
{
  return true;
  /*
  // turn off error printing
  H5E_auto2_t old_func;
  void *old_client_data;
  H5::Exception::getAutoPrint(old_func, &old_client_data);
  H5::Exception::dontPrint(); // turn off error printing

  // check if file exists
  hid_t file_hid;
  file_hid = H5Fopen(file_name, H5F_ACC_RDWR, H5P_DEFAULT);
  if (file_hid < 0)
  return false;
  H5Fclose(file_hid);

  H5::H5File file(file_name, H5F_ACC_RDWR);

  try
  {
  actnum_array.resize((it - is + 1) * (jt - js  + 1) * (kt - ks + 1));
  H5::DataSet dataset = file.openDataSet("activ");
  hsize_t dims_memory[] = {((it-is+1))*(jt - js + 1)};
  H5::DataSpace dataspace_memory(1, dims_memory);
  H5::DataSpace dataspace_file = dataset.getSpace();
  hsize_t count[] = {jt - js + 1};
  hsize_t stride[] = {nx};
  hsize_t block[] = {it - is + 1};
  for (int k = 0; k < kt - ks + 1; k++)
  {
  hsize_t start[] = {is + js * nx + (k + ks) * nx * ny};
  dataspace_file.selectHyperslab(H5S_SELECT_SET, count, start, stride, block);
  dataset.read(&actnum_array[k * count[0] * block[0]], H5::PredType::NATIVE_INT, dataspace_memory, dataspace_file);
  }
  }
  catch (H5::FileIException exception)
  {
  exception.printError();
  return false;
  }

  H5::Exception::setAutoPrint(old_func, old_client_data);
  n_active_elements = accumulate(actnum_array.begin(),actnum_array.end(),0);
  return true;
  */
}

template<class strategy_t>
bool mesh_grdecl<strategy_t>::file_open_cube_hdf5(const char* file_name, int is, int js, int ks, int it, int jt, int kt)
{
  // turn off error printing
  H5E_auto2_t old_func;
  void *old_client_data;
  H5::Exception::getAutoPrint(old_func, &old_client_data);
  H5::Exception::dontPrint();

  // check if file exists
  hid_t file_hid;
  file_hid = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (file_hid < 0)
    return false;
  H5Fclose(file_hid);

  H5::H5File file(file_name, H5F_ACC_RDONLY);

  try
    {
      // read "coord" array
      H5::DataSet dataset_coord = file.openDataSet("coord");
      hsize_t dims_memory_coord[] = {6 * (it - is + 2) * (jt - js  + 2)};
      H5::DataSpace dataspace_memory_coord(1, dims_memory_coord);
      H5::DataSpace dataspace_file_coord = dataset_coord.getSpace();
      coord_array.clear();
      vector<fp_type_t> buf_array(6 * (it - is + 2) * (jt - js  + 2));
      // hyperslab settings
      hsize_t count_coord[] = {jt - js + 2};
      hsize_t start_coord[] = {(is + js * (nx + 1)) * 6};
      hsize_t stride_coord[] = {6 * (nx + 1)};
      hsize_t block_coord[] = {(it - is + 2) * 6};
      // we read "coord" array by 1 read operation
      dataspace_file_coord.selectHyperslab(H5S_SELECT_SET, count_coord, start_coord, stride_coord, block_coord);
      dataset_coord.read(&buf_array[0], H5::PredType::NATIVE_FLOAT, dataspace_memory_coord, dataspace_file_coord);
      for (int i = 0; i < (it - is + 2) * (jt - js  + 2); i++)
        coord_array.push_back(coordElem (fpoint3d(buf_array[i*6],buf_array[i*6+1],buf_array[i*6+2]),
                                         fpoint3d(buf_array[i*6+3],buf_array[i*6+4],buf_array[i*6+5])));

      // read "zcorn" array
      H5::DataSet dataset_zcorn = file.openDataSet("zcorn");
      hsize_t dims_memory_zcorn_array[] = {8 * (it - is + 1) * (jt - js + 1)};
      H5::DataSpace dataspace_memory_zcorn(1, dims_memory_zcorn);
      H5::DataSpace dataspace_file_zcorn = dataset_zcorn.getSpace();
      zcorn_array.resize(8 * (it - is + 1) * (jt - js  + 1) * (kt - ks + 1));
      // hyperslab settings (common for each plane)
      hsize_t count_zcorn_array[] = {4 * (jt - js + 1)};
      hsize_t stride_zcorn_array[] = {2 * nx};
      hsize_t block_zcorn_array[] = {2 * (it - is + 1)};
      // we read "zcorn" array by planes - kt-ks+1 read operations
      for (int k = 0; k < kt - ks + 1; k++)
        {
          // determine start array for each plane individually
          hsize_t start_zcorn_array[] = {2 * is + 4 * js * nx + 8 * k * nx * ny};
          dataspace_file_zcorn.selectHyperslab(H5S_SELECT_SET, count_zcorn, start_zcorn, stride_zcorn, block_zcorn);
          dataset_zcorn.read(&zcorn_array[k * count_zcorn_array[0] * block_zcorn_array[0]], H5::PredType::NATIVE_FLOAT, dataspace_memory_zcorn, dataspace_file_zcorn);
        }
    }
  catch (H5::Exception exception)
    {
      exception.printError();
      coord_array.clear();
      zcorn_array.clear();
      return false;
    }

  // enable error printing
  H5::Exception::setAutoPrint(old_func, old_client_data);

  return true;
}

template<class strategy_t>
int mesh_grdecl<strategy_t>::append_array_hdf5(const fp_type_t *arr, size_t arr_length, H5::DataSet *dataset)
{
  // determine new dimensions of dataset
  hsize_t dims_old[1];
  hsize_t dims_memory[1] = {arr_length};
  H5::DataSpace dataspace_file = dataset->getSpace();
  dataspace_file.getSimpleExtentDims(dims_old);
  hsize_t dims_new[1] = {dims_old[0] + dims_memory[0]};
  // extend dataset
  dataset->extend(dims_new);
  dataspace_file = dataset->getSpace();
  H5::DataSpace dataspace_memory(1, dims_memory);
  // select hyperslab to write
  hsize_t count[1] = {dims_memory[0]};
  hsize_t start[1] = {dims_old[0]};
  dataspace_file.selectHyperslab(H5S_SELECT_SET, count, start);
  // write array
  dataset->write(arr, H5::PredType::NATIVE_FLOAT, dataspace_memory, dataspace_file);

  return 0;
}

template<class strategy_t>
bool mesh_grdecl<strategy_t>::file_open_cube_with_hdf5_swap(const char* file_name)
{
  try
    {
      fstream file(file_name, ios::in);
      if (!file.is_open())
        return false;

      H5::H5File file_hdf5(filename_hdf5, H5F_ACC_TRUNC);
      H5::DataSet *dataset_coords = 0;
      H5::DataSet *dataset_zcorn = 0;

      char buf[100];
      item_array_t buf_array;
      bool array_end;

      /////////////////////////////////
      // read "coord"
      /////////////////////////////////
      while (!file.eof())
        {
          file >> buf;
          if (strcmp(buf, "COORD") == 0)
            break;
        }
      create_array_hdf5("coord", file_hdf5, &dataset_coords);
      array_end = false;
      while (!file.eof())
        {
          // check if it is array end
          file >> buf;
          if (buf[0] == '/')
            array_end = true;

          if (array_end != true)
            {
              buf_array.push_back((float) atof(buf));
            }

          // if maximum space is used
          // or if it is array end
          // write "coords" to the swap file
          if ((buf_array.size() >= HDF5_MAX_SPACE)  ||
              ((array_end == true) && (buf_array.size() > 0))
             )
            {
              // save coord_array to hdf5 swap file
              append_array_hdf5(&buf_array[0], buf_array.size(), dataset_coords);
              buf_array.clear();
            }

          if (array_end == true)
            break;
        }
      delete dataset_coords;

      /////////////////////////////////
      // read "zcorn"
      /////////////////////////////////
      while (!file.eof())
        {
          file >> buf;

          if (strcmp(buf, "ZCORN") == 0)
            break;
        }
      // creating dataset_zcorn
      create_array_hdf5("zcorn", file_hdf5, &dataset_zcorn);
      array_end = false;
      while (!file.eof())
        {
          // check if it is array end
          file >> buf;
          if (buf[0] == '/')
            array_end = true;

          if (array_end != true)
            {
              buf_array.push_back((float) atof(buf));
            }

          // if maximum space is used
          // or if it is array end
          // write "coords" to the swap file
          if ((buf_array.size() >= HDF5_MAX_SPACE)  ||
              ((array_end == true) && (buf_array.size() > 0))
             )
            {
              // save coord_array to hdf5 swap file
              append_array_hdf5(&buf_array[0], buf_array.size(), dataset_zcorn);
              buf_array.clear();
            }

          if (array_end == true)
            break;
        }
      delete dataset_zcorn;
      return true;
    }
  catch (H5::Exception exception)
    {
      exception.printError();
      return false;
    }
}
#endif

template<class strategy_t>
bool mesh_grdecl<strategy_t>::file_open_actnum(const char* file_name)
{
#if 0
  fstream file(file_name,  ios::in);
  char buf[255];
  char *start_ptr,*end_ptr;
  if (!file.is_open())
    {
      n_active_elements =  accumulate(actnum_array.begin(),actnum_array.end(),0);
      return false;
    }
  actnum_array.clear();

  while (!file.eof())
    {
      file >> buf;
      start_ptr = buf;
      if (strcmp (buf, "/") != 0)
        actnum_array.push_back((int)strtod (start_ptr, &end_ptr));
      else
        break;
    }
  n_active_elements =  accumulate(actnum_array.begin(),actnum_array.end(),0);
#endif
  return true;
}


template<class strategy_t>
bool mesh_grdecl<strategy_t>::file_open_cube(const char* file_name)
{
  /*
  using namespace std;
  fstream file(file_name,  ios::in);
  char buf[100];
  char *start_ptr, *end_ptr;
  if (!file.is_open())
    return false;

  while (!file.eof())
    {
      file >> buf;
      if (  strcmp (buf, "COORD") == 0)
        break;
    }

  max_x = max_y = -100000;
  min_x = min_y = 1000000;

  i_type_t i_count = 0;
  const int max_count = 6; //buffer_size
  vector<float> buf_vec(max_count);
  while (!file.eof())
    {
      //coordElem curElem;
      file >> buf;

      if (buf[0] != '/')
        {
          buf_vec[i_count++] = (float)atof(buf);
          if (i_count == max_count) //swap_buffer
            {
              i_count = 0;
              coordElem curElem = coordElem(fpoint3d(buf_vec[0],buf_vec[1],buf_vec[2]),
                                            fpoint3d(buf_vec[3],buf_vec[4],buf_vec[5]));
              coord_array.push_back(curElem);

              //looking for max&min coordinates
              if (curElem.pStart.x > max_x)   max_x = curElem.pStart.x;
              if (curElem.pEnd.x > max_x)     max_x = curElem.pEnd.x;

              if (curElem.pStart.y > max_y)   max_y = curElem.pStart.y;
              if (curElem.pEnd.y > max_y)     max_y = curElem.pEnd.y;

              if (curElem.pStart.x < min_x)   min_x = curElem.pStart.x;
              if (curElem.pEnd.x < min_x)     min_x = curElem.pEnd.x;

              if (curElem.pStart.y < min_y)   min_y = curElem.pStart.y;
              if (curElem.pEnd.y < min_y)     min_y = curElem.pEnd.y;
            }
        }
      else
        break;
    }

  while (!file.eof())
    {
      file >> buf;
      if (  strcmp (buf, "ZCORN") == 0)
        break;
    }

  while (!file.eof())
    {
      file >> buf;
      start_ptr = buf;
      if (strcmp (buf, "/") != 0)
        zcorn_array.push_back((float)strtod (start_ptr, &end_ptr));
      else
        break;
    }
  min_z = *(std::min_element(zcorn_array.begin(),zcorn_array.end()));
  max_z = *(std::max_element(zcorn_array.begin(),zcorn_array.end()));
  return true;
  */
  return false;
}


BS_INST_STRAT(mesh_grdecl);
