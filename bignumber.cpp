void _div_uint48to32(unsigned int x[2], unsigned int _y, unsigned int q[2])
{
	q[0] = q[1] = 0;
	if (_y == 0) { x[0] = x[1] = 0;return; }
	if (x[1] == 0) { q[0] = x[0] / _y;x[0] -= q[0] * _y;x[1] = 0;return; }

	unsigned int c, d, ds, z, s = 0;
	unsigned short
		*y = (unsigned short*)(&_y);

	//выравниваем y
	if ((_y & 0x80000000UL) == 0) { s = _clz(_y);_y <<= s; }

	z = (x[1] << 16) | (x[0] >> 16);
	//1.
	c = z / y[1];z -= c * y[1];
	z = (z << 16) | (x[0] & 0xFFFFUL);
	d = c * y[0];
	//ds = c > 0x10000UL && 0x00010000UL >= -((c & 0xFFFFUL)) ? 1 : 0;
	ds = c > 0x10000UL && (c & 0xFFFFUL)*y[0] >= ((0x10000UL - y[0]) << 16) ? 1 : 0;
	if ((((uint64_t)c)*y[0]) >= 0x100000000ull)
		s = 0;
	if (ds == 0 && z >= d)
	{
		z -= d;if (z >= _y) { c++; z -= _y; }
	}
	else
	{
		if (d < z)ds--;
		z = d - z;c--;
		//приводим z < 2^32
		if (ds > 0)
		{
			if (z < _y)ds--;
			z -= _y;c--;
			if (ds > 0)
			{
				if (z < _y)ds--;
				z -= _y;c--;
			}
		}
		if (z > _y) { d = z / _y;c -= d;z -= d * _y; }
		z = _y - z;
	}
	c <<= s;
	q[0] = c;x[1] = 0;x[0] = z;
}
