void _div_uint48to32(unsigned int x[2], unsigned int _y, unsigned int q[2])
{
	q[0] = q[1] = 0;
	if (_y == 0) { x[0] = x[1] = 0;return; }
	if (x[1] == 0) { q[0] = x[0] / _y;x[0] -= q[0] * _y;x[1] = 0;return; }

	unsigned int c, d, d2, z, s = 0;
	unsigned short
		*y = (unsigned short*)(&_y);

	//выравниваем y
	if ((_y & 0x80000000UL) == 0) { s = _clz(_y);_y <<= s; }

	z = (x[1] << 16) | (x[0] >> 16);
	//1.
	c = z / y[1];z -= c * y[1];
	z = (z << 16) | (x[0] & 0xFFFFUL);
	d = c * y[0];
	d2 = c > 0x10000UL && (((c & 0xFFFFUL)*y[0]) >> 16) + y[0] >= 0x10000UL ? 1 : 0;
	if (d2 == 0 && z >= d)
	{
		z -= d;if (z >= _y) { c++; z -= _y; }
	}
	else
	{
		if (d2 == 0)
		{
			z = d - z;
		}
		else
		{//k*y >= [d2,d]-z    k=ceil(([d2,d]-z)/y);
			if (d < z)
			{
				z = d - z;
			}
			else
			{
				z = d - z;
				if (z >= _y) { c--;z -= _y; }
				c--;z -= _y;
			}
		}
		if (z > _y) { c--;z -= _y; }
		c--;z = _y - z;
	}
	c <<= s;
	q[0] = c;x[1] = 0;x[0] = z;
}
