#include "get_sort_indices.h"


struct special_comparer_struct {
	double val;
	long index;
};
struct special_comparer
{
	bool operator()(const special_comparer_struct & a, const special_comparer_struct & b) const {
		if (a.val<b.val) return true;
		else if (a.val==b.val) {
			return (a.index<b.index);
		}
		else return false;
	}
};

QList<long> get_sort_indices(const QList<long> &X) {
	QList<special_comparer_struct> list;
	for (long i=0; i<X.count(); i++) {
		special_comparer_struct tmp;
		tmp.val=X[i];
		tmp.index=i;
		list << tmp;
	}
	qSort(list.begin(),list.end(),special_comparer());
	QList<long> ret;
	for (int i=0; i<list.count(); i++) {
		ret << list[i].index;
	}
	return ret;
}

QList<long> get_sort_indices(const QList<double> &X) {
	QList<special_comparer_struct> list;
	for (long i=0; i<X.count(); i++) {
		special_comparer_struct tmp;
		tmp.val=X[i];
		tmp.index=i;
		list << tmp;
	}
	qSort(list.begin(),list.end(),special_comparer());
	QList<long> ret;
	for (int i=0; i<list.count(); i++) {
		ret << list[i].index;
	}
	return ret;
}
