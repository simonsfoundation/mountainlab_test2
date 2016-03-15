#include "diskarraymodel.h"
#include <stdio.h>
#include <QDebug>
#include <QMap>
#include <QFileInfo>
#include <QByteArray>
#include "sscommon.h"
#include <QDir>
#include <QProgressDialog>
#include <QApplication>
#include <QTime>
#include <QDateTime>
#include "mdaio.h"

class DiskArrayModelPrivate {
public:
	DiskArrayModel *q;
	QString m_path;
    long m_scale_factor;
    long m_chunk_size;
    long m_num_channels;
    long m_num_timepoints;
    long m_dim3;
    //long m_data_type;
	QMap<QString,Mda *> m_loaded_chunks;
	Mda m_mda;
	bool m_set_from_mda;
	void read_header();
    Mda *load_chunk(long scale,long chunk_ind);
    Mda *load_chunk_from_file(QString path,long scale,long chunk_ind);
    QVector<double> load_data_from_mda(QString path,long n,long i1,long i2,long i3);
    QString get_code(long scale,long chunk_size,long chunk_ind);
    QString get_multiscale_file_name(long scale);
	bool file_exists(QString path);
	void clean_up_file_hierarchies_in_path(const QString &path);
};

DiskArrayModel::DiskArrayModel(QObject *parent) : QObject(parent)
{
	d=new DiskArrayModelPrivate;
	d->q=this;

	d->m_set_from_mda=false;
	d->m_scale_factor=4;
	d->m_chunk_size=1;
	d->m_num_channels=1;
	d->m_num_timepoints=0;
	d->m_dim3=1;
	//d->m_data_type=MDAIO_TYPE_FLOAT32;
}

DiskArrayModel::~DiskArrayModel()
{
	foreach (Mda *X,d->m_loaded_chunks) {
		delete X;
	}
	delete d;
}

void DiskArrayModel::setPath(QString path) {
	d->m_path=path;
	d->read_header();

	d->clean_up_file_hierarchies_in_path(QFileInfo(d->m_path).path());

    d->m_chunk_size=(long)qMax(10000.0/d->m_num_channels,1000.0);
}

void DiskArrayModel::setFromMda(const Mda &X)
{
	d->m_mda=X;
	d->m_set_from_mda=true;
	d->m_num_channels=X.N1();
	d->m_num_timepoints=X.N2()*X.N3();
	d->m_dim3=X.N3();
}

QString DiskArrayModel::path()
{
	return d->m_path;
}

double safe_value1(Mda &X,long ii) {
	if (ii<X.totalSize()) return X.get(ii);
	else return 0;
}

Mda DiskArrayModel::loadData(long scale,long t1,long t2) {
    QTime timer; timer.start();

    Mda X;
    long d3=2;
	X.allocate(d->m_num_channels,t2-t1+1,d3);


	if (d->m_set_from_mda) {
        long M=d->m_mda.N1();

		if (scale==1) {
            for (long tt=t1; tt<=t2; tt++) {
                for (long mm=0; mm<d->m_num_channels; mm++) {
					X.setValue(safe_value1(d->m_mda,mm+tt*M),mm,tt-t1,0);
					X.setValue(safe_value1(d->m_mda,mm+tt*M),mm,tt-t1,1);
				}
			}
		}
		else {
            for (long tt=t1; tt<=t2; tt++) {
                for (long mm=0; mm<d->m_num_channels; mm++) {
                    double minval=safe_value1(d->m_mda,mm+tt*scale*M);
                    double maxval=safe_value1(d->m_mda,mm+tt*scale*M);
                    for (long dt=0; dt<scale; dt++) {
                        double tmpval=safe_value1(d->m_mda,mm+(tt*scale+dt)*M);
						if (tmpval<minval) minval=tmpval;
						if (tmpval>maxval) maxval=tmpval;
					}
					//X.setValue(d->m_mda.value1(mm+tt*M),mm,tt-t1,minval);
					//X.setValue(d->m_mda.value1(mm+tt*M),mm,tt-t1,maxval);
                    //oops!!! this problem was fixed on 11/5/2015
                    X.setValue(minval,mm,tt-t1,0);
                    X.setValue(maxval,mm,tt-t1,1);
				}
			}
		}
		return X;
	}

    long chunk_ind=t1/d->m_chunk_size;
    for (long i=chunk_ind; i*d->m_chunk_size<=t2; i++) {
        long tA1,tB1;
        long tA2,tB2;
		if (i>chunk_ind) {
			tA1=0; //where it's coming from
		}
		else {
			tA1=t1-i*d->m_chunk_size; //where it's coming from
		}

		if ((i+1)*d->m_chunk_size<=t2) {
			tB1=d->m_chunk_size-1; //where it's coming from
		}
		else {
			tB1=t2-i*d->m_chunk_size; //where it's coming from
		}
		tA2=tA1+i*d->m_chunk_size-t1; //where it's going to
		tB2=tB1+i*d->m_chunk_size-t1; //where it's going to
		Mda *C=d->load_chunk(scale,i);
		if (!C) return X;
        for (long dd=0; dd<d3; dd++) {
            for (long tt=tA2; tt<=tB2; tt++) {
                for (long ch=0; ch<d->m_num_channels; ch++) {
					X.setValue(C->value(ch,tt-tA2+tA1,dd),ch,tt,dd);
				}
			}
		}
	}

	return X;
}

double DiskArrayModel::value(long ch,long t) {
	if (d->m_set_from_mda) {
        long M=d->m_mda.N1();
		return safe_value1(d->m_mda,ch+t*M);
	}
	Mda tmp=loadData(1,t,t);
    return tmp.value(ch,0L);
}

bool DiskArrayModel::fileHierarchyExists() {
	if (d->m_set_from_mda) {
		return true;
	}
    long scale0=1;
    while (d->m_num_timepoints/(scale0*MULTISCALE_FACTOR)>1) scale0*=MULTISCALE_FACTOR;
	QString path0=d->get_multiscale_file_name(scale0);
	return (d->file_exists(path0));
}

bool do_minmax_downsample(QString path1,QString path2,long factor,QProgressDialog *dlg,bool first) {
	FILE *inf=jfopen(path1.toLatin1().data(),"rb");
	if (!inf) {
		qWarning() << "Problem reading data from mda for do_minmax_downsample: " << path1 << factor;
		return false;
	}
	FILE *outf=jfopen(path2.toLatin1().data(),"wb");
	if (!outf) {
		qWarning() << "Problem writing data to mda for do_minmax_downsample: " << path2 << factor;
		jfclose(inf);
		return false;
	}


	MDAIO_HEADER HH;
	mda_read_header(&HH,inf);
    long dim1=HH.dims[0];
    long dim2=HH.dims[1];
    long dim3=HH.dims[2];
	if (first) {
		dim2=dim2*dim3; dim3=1; //I know, it's a hack
	}
    long header_size=HH.header_size;
    long data_type=HH.data_type;

    long dim1b=dim1;
    long dim2b=dim2/factor; //if (dim2b*factor<dim2) dim2b++;
    long remainder=dim2-dim2b*factor;
    long dim3b=2;
    long num_dimsb=3;

	HH.dims[0]=dim1b;
	HH.dims[1]=dim2b;
	HH.dims[2]=dim3b;
	HH.num_dims=num_dimsb;
	mda_write_header(&HH,outf);

	QTime timer; timer.start();

	if (data_type==MDAIO_TYPE_BYTE) {
		unsigned char *buf=(unsigned char *)jmalloc(sizeof(unsigned char)*factor*dim1);
        for (long kk=0; kk<2; kk++) { //determines min or max
			if (dim3==1) {
				fseek(inf,header_size,SEEK_SET); //need to go back to first part in this case
			}
			for (long i=0; i<dim2b; i++) {
				if ((i%100==0)&&(timer.elapsed()>=500)) {
					timer.start();
					if (dlg->wasCanceled()) {
						QFile::remove(path2);
						return false;
					}
					dlg->setValue((i*50)/dim2b+50*kk);
					qApp->processEvents();
				}
                long numread=mda_read_byte(buf,&HH,factor*dim1,inf);
                //long numread=fread(buf,sizeof(unsigned char),factor*dim1,inf);
                unsigned char vals[dim1]; for (long a=0; a<dim1; a++) vals[a]=0;
				if (numread<factor*dim1) {
					qWarning() << "Warning, unexpected problem in do_minmax_downsample" << kk << i << numread << factor << dim1;
				}
                for (long ch=0; ch<dim1; ch++) {
					vals[ch]=buf[0*dim1+ch];
                    for (long j=0; j<numread/dim1; j++) {
						if (kk==0) vals[ch]=qMin(vals[ch],buf[j*dim1+ch]);
						else if (kk==1) vals[ch]=qMax(vals[ch],buf[j*dim1+ch]);
					}
				}
				mda_write_byte(vals,&HH,dim1,outf);
				//fwrite(vals,sizeof(unsigned char),dim1,outf);
			}
			if (remainder>0) {
                long numread=mda_read_byte(buf,&HH,remainder*dim1,inf);
                //long b0=fread(buf,sizeof(unsigned char),remainder*dim1,inf);
				Q_UNUSED(numread);
			}
		}
		jfree(buf);
	}
	else if (data_type==MDAIO_TYPE_INT32) {
		qint32 *buf=(qint32 *)jmalloc(sizeof(qint32)*factor*dim1);
		for (int kk=0; kk<2; kk++) { //determines min or max
			if (dim3==1) {
				fseek(inf,header_size,SEEK_SET); //need to go back to first part in this case
			}
			for (long i=0; i<dim2b; i++) {
				if ((i%100==0)&&(timer.elapsed()>=500)) {
					timer.start();
					if (dlg->wasCanceled()) {
						QFile::remove(path2);
						return false;
					}
					dlg->setValue((i*50)/dim2b+50*kk);
					qApp->processEvents();
				}
				int numread=mda_read_int32(buf,&HH,factor*dim1,inf);
				//int numread=fread(buf,sizeof(qint32),factor*dim1,inf);
				qint32 vals[dim1]; for (int a=0; a<dim1; a++) vals[a]=0;
				if (numread<factor*dim1) {
					qWarning() << "Warning, unexpected problem in do_minmax_downsample" << kk << i << numread << factor << dim1;
				}
				for (int ch=0; ch<dim1; ch++) {
					vals[ch]=buf[0*dim1+ch];
					for (int j=0; j<numread/dim1; j++) {
						if (kk==0) vals[ch]=qMin(vals[ch],buf[j*dim1+ch]);
						else if (kk==1) vals[ch]=qMax(vals[ch],buf[j*dim1+ch]);
					}
				}
				mda_write_int32(vals,&HH,dim1,outf);
				//fwrite(vals,sizeof(qint32),dim1,outf);
			}
			if (remainder>0) {
				int numread=mda_read_int32(buf,&HH,remainder*dim1,inf);
				//int b0=fread(buf,sizeof(qint32),remainder*dim1,inf);
				Q_UNUSED(numread)
			}
		}
		jfree(buf);
	}
	else if (data_type==MDAIO_TYPE_FLOAT32) {
		float *buf=(float *)jmalloc(sizeof(float)*factor*dim1);
		for (int kk=0; kk<2; kk++) { //determines min or max
			if (dim3==1) {
				fseek(inf,header_size,SEEK_SET); //need to go back to first part in this case
			}
			for (long i=0; i<dim2b; i++) {
				if ((i%100==0)&&(timer.elapsed()>=500)) {
					timer.start();
					if (dlg->wasCanceled()) {
						QFile::remove(path2);
						return false;
					}
					dlg->setValue((i*50)/dim2b+50*kk);
					qApp->processEvents();
				}
				int numread=mda_read_float32(buf,&HH,factor*dim1,inf);
				//int numread=fread(buf,sizeof(float),factor*dim1,inf);
				float vals[dim1]; for (int a=0; a<dim1; a++) vals[a]=0;
				if (numread<factor*dim1) {
					qWarning() << "Warning, unexpected problem in do_minmax_downsample" << kk << i << numread << factor << dim1;
				}
				for (int ch=0; ch<dim1; ch++) {
					vals[ch]=buf[0*dim1+ch];
					for (int j=0; j<numread/dim1; j++) {
						if (kk==0) vals[ch]=qMin(vals[ch],buf[j*dim1+ch]);
						else if (kk==1) vals[ch]=qMax(vals[ch],buf[j*dim1+ch]);
					}
				}
				mda_write_float32(vals,&HH,dim1,outf);
				//fwrite(vals,sizeof(float),dim1,outf);
			}
			if (remainder>0) {
				int numread=mda_read_float32(buf,&HH,remainder*dim1,inf);
				//int b0=fread(buf,sizeof(float),remainder*dim1,inf);
				Q_UNUSED(numread)
			}
		}
		jfree(buf);
	}
	else if (data_type==MDAIO_TYPE_FLOAT64) {
        double *buf=(double *)jmalloc(sizeof(double)*factor*dim1);
        for (int kk=0; kk<2; kk++) { //determines min or max
            if (dim3==1) {
                fseek(inf,header_size,SEEK_SET); //need to go back to first part in this case
            }
            for (long i=0; i<dim2b; i++) {
                if ((i%100==0)&&(timer.elapsed()>=500)) {
                    timer.start();
                    if (dlg->wasCanceled()) {
                        QFile::remove(path2);
                        return false;
                    }
                    dlg->setValue((i*50)/dim2b+50*kk);
                    qApp->processEvents();
                }
                int numread=mda_read_float64(buf,&HH,factor*dim1,inf);
                //int numread=fread(buf,sizeof(double),factor*dim1,inf);
                double vals[dim1]; for (int a=0; a<dim1; a++) vals[a]=0;
                if (numread<factor*dim1) {
                    qWarning() << "Warning, unexpected problem in do_minmax_downsample" << kk << i << numread << factor << dim1;
                }
                for (int ch=0; ch<dim1; ch++) {
                    vals[ch]=buf[0*dim1+ch];
                    for (int j=0; j<numread/dim1; j++) {
                        if (kk==0) vals[ch]=qMin(vals[ch],buf[j*dim1+ch]);
                        else if (kk==1) vals[ch]=qMax(vals[ch],buf[j*dim1+ch]);
                    }
                }
                mda_write_float64(vals,&HH,dim1,outf);
                //fwrite(vals,sizeof(float),dim1,outf);
            }
            if (remainder>0) {
                int numread=mda_read_float64(buf,&HH,remainder*dim1,inf);
                //int b0=fread(buf,sizeof(float),remainder*dim1,inf);
                Q_UNUSED(numread)
            }
        }
        jfree(buf);
    }
	else if (data_type==MDAIO_TYPE_INT16) {
		qint16 *buf=(qint16 *)jmalloc(sizeof(qint16)*factor*dim1);
		for (int kk=0; kk<2; kk++) { //determines min or max
			if (dim3==1) {
				fseek(inf,header_size,SEEK_SET); //need to go back to first part in this case
			}
			for (long i=0; i<dim2b; i++) {
				if ((i%100==0)&&(timer.elapsed()>=500)) {
					timer.start();
					if (dlg->wasCanceled()) {
						QFile::remove(path2);
						return false;
					}
					dlg->setValue((i*50)/dim2b+50*kk);
					qApp->processEvents();
				}
				int numread=mda_read_int16(buf,&HH,factor*dim1,inf);
				//int numread=fread(buf,sizeof(qint16),factor*dim1,inf);
				qint16 vals[dim1]; for (int a=0; a<dim1; a++) vals[a]=0;
				if (numread<factor*dim1) {
					qWarning() << "Warning, unexpected problem in do_minmax_downsample" << kk << i << numread << factor << dim1;
				}
				for (int ch=0; ch<dim1; ch++) {
					vals[ch]=buf[0*dim1+ch];
					for (int j=0; j<numread/dim1; j++) {
						if (kk==0) vals[ch]=qMin(vals[ch],buf[j*dim1+ch]);
						else if (kk==1) vals[ch]=qMax(vals[ch],buf[j*dim1+ch]);
					}
				}
				mda_write_int16(vals,&HH,dim1,outf);
				//fwrite(vals,sizeof(qint16),dim1,outf);
			}
			if (remainder>0) {
				int numread=mda_read_int16(buf,&HH,remainder*dim1,inf);
				//int b0=fread(buf,sizeof(qint16),remainder*dim1,inf);
				Q_UNUSED(numread)
			}
		}
		jfree(buf);
	}
	else if (data_type==MDAIO_TYPE_UINT16) {
		quint16 *buf=(quint16 *)jmalloc(sizeof(quint16)*factor*dim1);
		for (int kk=0; kk<2; kk++) { //determines min or max
			if (dim3==1) {
				fseek(inf,header_size,SEEK_SET); //need to go back to first part in this case
			}
			for (long i=0; i<dim2b; i++) {
				if ((i%100==0)&&(timer.elapsed()>=500)) {
					timer.start();
					if (dlg->wasCanceled()) {
						QFile::remove(path2);
						return false;
					}
					dlg->setValue((i*50)/dim2b+50*kk);
					qApp->processEvents();
				}
				int numread=mda_read_uint16(buf,&HH,factor*dim1,inf);
				//int numread=fread(buf,sizeof(quint16),factor*dim1,inf);
				quint16 vals[dim1]; for (int a=0; a<dim1; a++) vals[a]=0;
				if (numread<factor*dim1) {
					qWarning() << "Warning, unexpected problem in do_minmax_downsample" << kk << i << numread << factor << dim1;
				}
				for (int ch=0; ch<dim1; ch++) {
					vals[ch]=buf[0*dim1+ch];
					for (int j=0; j<numread/dim1; j++) {
						if (kk==0) vals[ch]=qMin(vals[ch],buf[j*dim1+ch]);
						else if (kk==1) vals[ch]=qMax(vals[ch],buf[j*dim1+ch]);
					}
				}
				mda_write_uint16(vals,&HH,dim1,outf);
				//fwrite(vals,sizeof(quint16),dim1,outf);
			}
			if (remainder>0) {
				int numread=mda_read_uint16(buf,&HH,remainder*dim1,inf);
				//int b0=fread(buf,sizeof(quint16),remainder*dim1,inf);
				Q_UNUSED(numread)
			}
		}
		jfree(buf);
	}
	else {
		qWarning() << "Unsupported data type in do_minmax_downsample" << data_type;
	}

	jfclose(outf);
	jfclose(inf);

	return true;
}

void DiskArrayModel::createFileHierarchyIfNeeded() {
	if (d->m_set_from_mda) return;
    if (fileHierarchyExists()) return;

	QProgressDialog dlg("Creating file hierarchy","Cancel",0,100);
	dlg.show();
	qApp->processEvents();

	QString last_path=d->m_path;
    long scale0=MULTISCALE_FACTOR;
	while (d->m_num_timepoints/scale0>1) {
		dlg.setLabelText(QString("Creating file hierarchy %1").arg(d->m_num_timepoints/scale0));
        qApp->processEvents();
        QString new_path=d->get_multiscale_file_name(scale0);
        if (!QDir(QFileInfo(new_path).path()).exists()) {
            //the following line is crazy!!
            QDir(QFileInfo(QFileInfo(new_path).path()).path()).mkdir(QFileInfo(QFileInfo(new_path).path()).fileName());
        }
        if (!do_minmax_downsample(last_path,new_path,MULTISCALE_FACTOR,&dlg,(scale0==MULTISCALE_FACTOR))) {
            exit(-1);
            return;
        }
        last_path=new_path;
		scale0*=MULTISCALE_FACTOR;
	}

	// This was the method that involves loading the whole thing into memory

	/*
	Mda X; X.read(d->m_path.toLatin1().data());
	int i3=0;
	int scale0=MULTISCALE_FACTOR;
	while (d->m_num_timepoints/scale0>1) {
		int N2=X.size(1)/MULTISCALE_FACTOR; if (N2*MULTISCALE_FACTOR<X.size(1)) N2++;
		Mda X2; X2.setDataType(d->m_data_type);
		X2.allocate(d->m_num_channels,N2,2);
		for (int tt=0; tt<N2; tt++) {
			for (int ch=0; ch<d->m_num_channels; ch++) {
				float minval=X.value(ch,tt*MULTISCALE_FACTOR,0);
				float maxval=X.value(ch,tt*MULTISCALE_FACTOR,i3);
				for (int dt=0; dt<MULTISCALE_FACTOR; dt++) {
					minval=qMin(minval,X.value(ch,tt*MULTISCALE_FACTOR+dt,0));
					maxval=qMax(maxval,X.value(ch,tt*MULTISCALE_FACTOR+dt,i3));
				}
				X2.setValue(minval,ch,tt,0);
				X2.setValue(maxval,ch,tt,1);
			}
		}
		QString path0=d->get_multiscale_file_name(scale0);
		X2.write(path0.toLatin1().data());
		i3=1;
		scale0*=MULTISCALE_FACTOR;
		X=X2;
	}
	*/
}

void DiskArrayModelPrivate::read_header() {
	FILE *inf=jfopen(m_path.toLatin1().data(),"rb");
	if (!inf) {
		qWarning() << "Problem opening file: " << m_path;
		return;
	}


	MDAIO_HEADER HH;
	mda_read_header(&HH,inf);
    long dim1=HH.dims[0];
    long dim2=HH.dims[1];
    long dim3=HH.dims[2];
    //long data_type=HH.data_type;

	jfclose(inf);

    //m_data_type=data_type;
	m_num_channels=dim1;
	m_num_timepoints=dim2*dim3;
	m_dim3=dim3;
}
QString DiskArrayModelPrivate::get_code(long scale,long chunk_size,long chunk_ind) {
	return QString("%1-%2-%3").arg(scale).arg(chunk_size).arg(chunk_ind);
}

Mda *DiskArrayModelPrivate::load_chunk(long scale,long chunk_ind) {
	QString code=get_code(scale,m_chunk_size,chunk_ind);
	if (m_loaded_chunks.contains(code)) {
		return m_loaded_chunks[code];
	}
	//printf("Loading chunk %d/%d\n",scale,chunk_ind); QTime timer; timer.start();
	QString path=get_multiscale_file_name(scale);
	Mda *X=load_chunk_from_file(path,scale,chunk_ind);
	if (X) {
		//qDebug()  << "################################  Loaded chunk"  << scale << chunk_ind << X->totalSize()*4*1.0/1000000;
		m_loaded_chunks[code]=X;
	}
	return X;

}

Mda *DiskArrayModelPrivate::load_chunk_from_file(QString path,long scale,long chunk_ind) {
	if (scale==1) {
        QVector<double> d0=load_data_from_mda(path,m_num_channels*m_chunk_size,0,chunk_ind*m_chunk_size,0);
		if (d0.isEmpty()) return 0;
		Mda *ret=new Mda();
        //ret->setDataType(m_data_type);
		ret->allocate(m_num_channels,m_chunk_size,2);
        long ct=0;
        for (long tt=0; tt<m_chunk_size; tt++) {
            for (long ch=0; ch<m_num_channels; ch++) {
                double val=0;
				if (ct<d0.count()) val=d0[ct];
				ret->setValue(val,ch,tt,0);
				ret->setValue(val,ch,tt,1);
				ct++;
			}
		}
		return ret;
	}
	else {
        QVector<double> d0=load_data_from_mda(path,m_num_channels*m_chunk_size,0,chunk_ind*m_chunk_size,0);
        QVector<double> d1=load_data_from_mda(path,m_num_channels*m_chunk_size,0,chunk_ind*m_chunk_size,1);

		if (d0.isEmpty()) return 0;
		Mda *ret=new Mda();
        //ret->setDataType(m_data_type);
		ret->allocate(m_num_channels,m_chunk_size,2);
        long ct=0;
        for (long tt=0; tt<m_chunk_size; tt++) {
            for (long ch=0; ch<m_num_channels; ch++) {
                double val0=0,val1=0;
				if (ct<d0.count()) val0=d0[ct];
				if (ct<d1.count()) val1=d1[ct];
				ret->setValue(val0,ch,tt,0);
				ret->setValue(val1,ch,tt,1);

				ct++;
			}
		}

		return ret;
	}

}

QVector<double> DiskArrayModelPrivate::load_data_from_mda(QString path,long n,long i1,long i2,long i3) {
    QVector<double> ret;
	FILE *inf=jfopen(path.toLatin1().data(),"rb");
	if (!inf) {
		qWarning() << "Problem reading data from mda: " << path << i1 << i2 << i3 << n;
		return ret;
	}

	MDAIO_HEADER HH;
	mda_read_header(&HH,inf);
    long dim1=HH.dims[0];
    long dim2=HH.dims[1];
    long header_size=HH.header_size;
    long data_type=HH.data_type;
    long num_bytes=HH.num_bytes_per_entry;

    long i0=i1+i2*dim1+i3*dim1*dim2;
	fseek(inf,header_size+num_bytes*i0,SEEK_SET);
	ret.resize(n);
	if (data_type==MDAIO_TYPE_BYTE) {
		unsigned char *X=(unsigned char *)jmalloc(sizeof(unsigned char)*n);
        long b0=mda_read_byte(X,&HH,n,inf);
        //long b0=fread(X,sizeof(unsigned char),n,inf);
		Q_UNUSED(b0)
        for (long i=0; i<n; i++) {
            ret[i]=(double)X[i];
		}
		jfree(X);
	}
	else if (data_type==MDAIO_TYPE_INT32) {
		qint32 *X=(qint32 *)jmalloc(sizeof(qint32)*n);
		int b0=mda_read_int32(X,&HH,n,inf);
		//int b0=fread(X,sizeof(qint32),n,inf);
		Q_UNUSED(b0)
		for (int i=0; i<n; i++) {
            ret[i]=(double)X[i];
		}
		jfree(X);
	}
	else if (data_type==MDAIO_TYPE_FLOAT32) {
		float *X=(float *)jmalloc(sizeof(float)*n);
		int bytes_read=mda_read_float32(X,&HH,n,inf);
		//int bytes_read=fread(X,sizeof(float),n,inf);
		Q_UNUSED(bytes_read);
		for (int i=0; i<n; i++) {
            ret[i]=(double)X[i];
		}
		jfree(X);
	}
	else if (data_type==MDAIO_TYPE_FLOAT64) {
        double *X=(double *)jmalloc(sizeof(double)*n);
        int bytes_read=mda_read_float64(X,&HH,n,inf);
        //int bytes_read=fread(X,sizeof(double),n,inf);
        Q_UNUSED(bytes_read);
        for (int i=0; i<n; i++) {
            ret[i]=(double)X[i];
        }
        jfree(X);
    }
	else if (data_type==MDAIO_TYPE_INT16) {
		short *X=(short *)jmalloc(sizeof(short)*n);
		int bytes_read=mda_read_int16(X,&HH,n,inf);
		//int bytes_read=fread(X,sizeof(short),n,inf);
		Q_UNUSED(bytes_read)
		for (int i=0; i<n; i++) {
            ret[i]=(double)X[i];
		}
		jfree(X);
	}
	else if (data_type==MDAIO_TYPE_UINT16) {
		quint16 *X=(quint16 *)jmalloc(sizeof(quint16)*n);
		int bytes_read=mda_read_uint16(X,&HH,n,inf);
		//int bytes_read=fread(X,sizeof(quint16),n,inf);
		Q_UNUSED(bytes_read);
		for (int i=0; i<n; i++) {
            ret[i]=(double)X[i];
		}
		jfree(X);
	}
	else {
		qWarning() << "Unsupported data type: " << data_type;
		ret.clear();
	}

	jfclose(inf);
	return ret;
}
long DiskArrayModel::size(long dim) {
	if (dim==0) return d->m_num_channels;
	else if (dim==1) return d->m_num_timepoints;
	else return 1;
}

long DiskArrayModel::dim3() {
	return d->m_dim3;
}

QString get_timestamp(QDateTime time) {
	return time.toString("yyyy-mm-dd-hh-mm-ss");
}

QString DiskArrayModelPrivate::get_multiscale_file_name(long scale) {
	QString str=QString(".%1").arg(scale);
	QDateTime time=QFileInfo(m_path).lastModified();
	QString timestamp=get_timestamp(time);
	QString subdir="spikespy."+QFileInfo(m_path).baseName()+"."+timestamp+"/";
	if (scale==1) {
		str="";
		subdir="";
	}
	return QFileInfo(m_path).path()+"/"+subdir+QFileInfo(m_path).baseName()+str+".mda";
}
bool DiskArrayModelPrivate::file_exists(QString path) {
	return QFileInfo(path).exists();
}

void remove_spikespy_directory(QString path) {
	//for safety!!
	if (!QFileInfo(path).fileName().startsWith("spikespy.")) {
	qWarning() << "This is dangerous, tried to remove: " << path;
	return;
	}

	QStringList list=QDir(path).entryList(QStringList("*.mda"),QDir::Files,QDir::Name);
	foreach (QString fname,list) {
	if (!QFile::remove(path+"/"+fname)) {
		qWarning() << "Problem removing file: " << fname;
	}
	}
	if (!QDir(QFileInfo(path).path()).rmdir(QFileInfo(path).fileName())) {
		qWarning() << "Problem removing directory: " << path;
	}
}

void DiskArrayModelPrivate::clean_up_file_hierarchies_in_path(const QString &path)
{
	QStringList list=QDir(path).entryList(QStringList("spikespy.*"));
	foreach (QString dirname,list) {
    long ind1=dirname.indexOf(".");
    long ind2=dirname.lastIndexOf(".");
	if ((ind1>0)&&(ind2>ind1)) {
		QString base_name=dirname.mid(ind1+1,ind2-ind1-1);
		QString timestamp0=dirname.mid(ind2+1);
		bool okay=false;
		QString file_path=path+"/"+base_name+".mda";
		if (QFile::exists(file_path)) {
		QDateTime time0=QFileInfo(file_path).lastModified();
		QString timestamp1=get_timestamp(time0);
		if (timestamp0==timestamp1) okay=true;
		}
		if (!okay) {
		QString full_path=path+"/"+dirname;
		printf("Removing directory: %s\n",full_path.toLatin1().data());
		remove_spikespy_directory(full_path);
		}
	}
	}
}
