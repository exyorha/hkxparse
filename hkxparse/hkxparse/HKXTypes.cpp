#include <hkxparse/HKXTypes.h>
#include <hkxparse/Deserializer.h>

namespace hkxparse {
	Deserializer &operator >>(Deserializer &stream, HKXVector4 &val) {
		return stream >> val.x >> val.y >> val.z >> val.w;
	}

	Deserializer &operator >>(Deserializer &stream, HKXQuaternion &val) {
		return stream >> val.vec;
	}

	Deserializer &operator >>(Deserializer &stream, HKXMatrix3 &val) {
		return stream >> val.v[0] >> val.v[1] >> val.v[2];
	}

	Deserializer &operator >>(Deserializer &stream, HKXQsTransform &val) {
		return stream >> val.translation >> val.rotation >> val.scale;
	}

	Deserializer &operator >>(Deserializer &stream, HKXMatrix4 &val) {
		return stream >> val.v[0] >> val.v[1] >> val.v[2] >> val.v[3];
	}
}
