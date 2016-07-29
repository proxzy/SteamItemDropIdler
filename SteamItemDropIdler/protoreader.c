
unsigned int ReadProtoNumber( const unsigned char* pubInput, void* pubDest, unsigned int cubSize, bool bFixed = false )
{
	if ( bFixed )
	{
		switch ( cubSize )
		{
			case 1:
				*(char*)pubDest = *(char*)pubInput;
				break;
			case 2:
				*(short*)pubDest = *(short*)pubInput;
				break;
			case 4:
				*(int*)pubDest = *(int*)pubInput;
				break;
			case 8:
				*(long long*)pubDest = *(long long*)pubInput;
				break;
			default:
				return 0;
		}

		return cubSize;
	}

	char result8 = 0;
	short result16 = 0;
	int result32 = 0;
	long long result64 = 0;
	unsigned int bytes_read = 0;

	while ( bytes_read <= cubSize )
	{
		switch ( cubSize )
		{
			case 1:
				result8 |= ( pubInput[bytes_read] & 0x7F ) << ( bytes_read * 7 );
				break;
			case 2:
				result16 |= ( pubInput[bytes_read] & 0x7F ) << ( bytes_read * 7 );
				break;
			case 4:
				result32 |= ( pubInput[bytes_read] & 0x7F ) << ( bytes_read * 7 );
				break;
			case 8:
				result64 |= (long long)( pubInput[bytes_read] & 0x7F ) << ( bytes_read * 7 );
				break;
			default:
				return 0;
		}
		if ( ( pubInput[bytes_read++] & 0x80 ) == 0 ) {
			break;
		}
	}

	switch ( cubSize )
	{
		case 1:
			*(char*)pubDest = result8;
			break;
		case 2:
			*(short*)pubDest = result16;
			break;
		case 4:
			*(int*)pubDest = result32;
			break;
		case 8:
			*(long long*)pubDest = result64;
			break;
		default:
			return 0;
	}

	return bytes_read;
}
