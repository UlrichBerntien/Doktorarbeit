#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

enum { SAMPLES = 500, STARTSAMPLES = 20, MAXMESSUNG = 100 };


/////////////////////////////////////////////////////////////////////////////////


class CInFile
  {
private:
	FILE* m_fh;
	const char* m_name;
	int m_line;
public:
  CInFile ();
	void error ( const char* wo ) const;
	void open ( const char* fname );
  void close ();
	void readln ( char *const buffer, const int size );
	};

/////////////////////////////////////////////////////////////////////////////////

CInFile::CInFile ()
  : m_fh( NULL )
	, m_name( NULL )
	{
	}


void CInFile::error ( const char* wo ) const
	{
	fprintf( stderr, "Fehler in CInFile aufgetreten.\nUrsache: \"%s\".\n", wo );
	if( m_fh != NULL && m_name != NULL )
		{
		fprintf( stderr, "File \"%s\", Zeile %d\n", m_name, m_line );
		}
	exit( EXIT_FAILURE );
	}


void CInFile::open ( const char* fname )
	{
	m_name = strdup( fname );
	if( m_name == NULL ) error( "open, strdup" );
	m_fh = fopen( m_name, "r" );
	if ( m_fh == NULL ) error( "open" );
	m_line = 0;
	}


void CInFile::close ()
	{
	if( fclose( m_fh ) != 0 ) error( "fclose" );
	m_fh = NULL;
	m_name = NULL;
	}


void CInFile::readln ( char *const buffer, const int size )
	{
	if( fgets( buffer, size, m_fh ) == NULL )
	  buffer[0] = '\0';
  else
	  ++m_line;
	}


/////////////////////////////////////////////////////////////////////////////////


class CKanal
  {
private:
	double x [SAMPLES];
public:
	double& operator[] ( const int i ) { return x[i]; }
	double flaeche () const;
	double max ( const int upto = SAMPLES ) const;
	void sub ( const CKanal& );
	void nullen ();
	void invert ();
	void normierenFlaeche ();
	void normierenMax ( const int upto = SAMPLES );
	};


//
//	Flaeche unter der Kurve bestimmen
//
double CKanal::flaeche () const
	{
	double sum = fabs( x[0] );
	for( int i = 1; i < SAMPLES; ++i )
	  sum += fabs( x[i] );
	return sum;
	}


//
//  Maximum von 0 .. upto
//
double CKanal::max ( const int upto ) const
	{
	double akku = x[0];
	for( int i = 1; i < upto; ++i )
	  if( akku < x[i] ) akku = x[i];
	return akku;
	}


//
//  einen Kanal subtrahieren
//
void CKanal::sub ( const CKanal& s )
	{
	for( int i = 0; i < SAMPLES; ++i )
		x[i] -= s.x[i];
	}


//
//  Kanal verschieben, die ersten STARTSAMPLES Samples im auf 0
//
void CKanal::nullen ()
	{
	double mittel = x[0];
	int i;
  for( i = 1; i < STARTSAMPLES; ++i )
	  mittel += x[i];
	mittel /= STARTSAMPLES;
	for( i = 0; i < SAMPLES; ++i )
		x[i] -= mittel;
	}


void CKanal::invert ()
	{
	for( int i = 0; i < SAMPLES; ++i )
	  x[i] *= -1;
	}


//
//  normieren auf Fläche = 1 
//
void CKanal::normierenFlaeche ()
	{
	const double area = flaeche();
	if( fabs( area ) > 1e-10 )
	  for( int i = 0; i < SAMPLES; ++i )
		  x[i] /= area;
	else
		fputs( "Warnung: Flaeche fast Null.\n", stderr );
	}


//
//	normieren auf Max = 1
//
void CKanal::normierenMax ( const int upto )
	{
	const double maximum = max( upto );
	if( fabs( maximum ) > 1e-10 )
		for( int i = 0; i < SAMPLES; ++i )
			x[i] /= maximum;
	else
		fputs( "Warnung: Maximum fast Null.\n", stderr );
	}


/////////////////////////////////////////////////////////////////////////////////


class CMessung
  {
private:
	CKanal m_pm;
	CKanal m_u;
	CKanal m_idot;
public:
  void read ( CInFile& input );
	void nullen ();
	void sub ( const CMessung& );
	void normieren ();
	double kanal( const int k, const int t );
	};


void CMessung::read ( CInFile& input )
	{
	char buffer [128];
	for( int i = 0; i < SAMPLES; ++i )
		{
		int t;
		double pm, u, idot;
		input.readln( buffer, sizeof buffer );
		if( sscanf( buffer, "%d %lf %lf %lf", &t, &pm, &u, &idot ) != 4 )
		  input.error( "falsches Format in Messung" );
		if( t != i * 2 )
			input.error( "falsche Zeit im Sample" );
		m_pm[i] = pm;
		m_u[i] = u;
		m_idot[i] = idot;
		}
	}


//
//	Messungen auf Null beziehen
//	d.h. Mittelwert der ersten STARTSAMPLES auf 0 setzten
//
void CMessung::nullen ()
	{
	m_pm.nullen();
	m_u.nullen();
	m_idot.nullen();
	}


//
//  Eine Messung subtrahieren
//	z.B. für Null-Messung subtrahieren
//
void CMessung::sub ( const CMessung& x )
	{
	m_pm.sub( x.m_pm );
	m_u.sub( x.m_u );
	m_idot.sub( x.m_idot );
	}


//
//  Messung normieren:
//  - PM auf Flaeche = 1
//  - IDOT auf Maximum in erster Hälfte = 1
//
void CMessung::normieren ()
	{
	m_pm.invert();
	m_pm.normierenFlaeche();
//	m_idot.normierenMax( SAMPLES/2 );
//	m_u.normierenMax( SAMPLES/2 );
	}


double CMessung::kanal ( const int k, const int t )
	{
	double x;
	switch( k )
		{
	case 0:
		x = m_pm[t];
		break;
	case 1:
		x = m_u[t];
		break;
	case 2:
		x = m_idot[t];
		};
	return x;
	}

/////////////////////////////////////////////////////////////////////////////////


//
//	Programm mit Fehlermeldung beenden
//
void error ( const char* msg )
	{
	fputs( msg, stderr );
	fputc( '\n', stderr );
	exit( EXIT_FAILURE );
	}


//
//	Datei mit Nullmessung
//
static const char fnameNull [] = "Null-Messung.dat";


//
//	Dateinamen für die Dateien mit allen Messungen
//
static const char* fnameAlle [3] =
	{
	"PM-alle.dat",
	"U-alle.dat",
	"IDOT-alle.dat"
	};


//
//	Dateinamen für die Dateien mit dem Mittelwerten über alle Messungen
//
static const char* fnameMittel [3] =
	{
	"PM-mittel.dat",
	"U-mittel.dat",
	"IDOT-mittel.dat"
	};


int main ( int argc, char* argv [] )
	{
  int i, t;

	if( argc != 2 )
	  {
		fputs( "Aufrufformat: combi <datei>\n", stderr );
		exit( EXIT_FAILURE );
		}

	CMessung nullMessung;

  int messungen = 0;
	CMessung* messung [MAXMESSUNG];

  CInFile input;
	char buffer[128];

	// Null-Messung einlesen
	printf( "Null-Messung einlesen, Datei: \"%s\"\n", fnameNull );
	input.open( fnameNull );
	nullMessung.read( input );
	input.close();
	nullMessung.nullen();

	// Messungen einlesen
	input.open( argv[1] );
	do{
		input.readln( buffer, sizeof buffer );
		if( buffer[0] == 't' )
		  {
			messung[messungen] = new CMessung;
			if( messung[messungen] == NULL ) error( "new fail" );
			messung[messungen]->read( input );
			putchar( '.' );
			++messungen;
			if( messungen > MAXMESSUNG ) input.error( "zu viele Messungen" );
			}
		}
		while( buffer[0] == 't' );
	input.close();
	printf( "\n%d Messungen eingelesen.\n", messungen );

	// Signale vorbereiten	
  for( i = 0; i < messungen; ++i ) messung[i]->sub( nullMessung );
  for( i = 0; i < messungen; ++i ) messung[i]->nullen();
  for( i = 0; i < messungen; ++i ) messung[i]->normieren();
	puts( "Alle Signale normiert" );

	// Alle Messungen pro Kanal ausgeben
	for( int k = 0; k < 3; ++k )
		{
		FILE* output;
		output = fopen( fnameAlle[k], "w" );
		for( t = 0; t < SAMPLES; ++t )
			{
			fprintf( output, "%d", t * 2 );
			for( i = 0; i < messungen; ++i )
				fprintf( output, "\t%lf", messung[i]->kanal(k,t) );
			fputc( '\n', output );
			}
		fclose( output );
		}

	// Mittelwert über alle Messungen pro Kanal ausgeben
	for( k = 0; k < 3; ++k )
		{
		FILE* output;
		output = fopen( fnameMittel[k], "w" );
		for( t = 0; t < SAMPLES; ++t )
			{
			double akku = 0;
			for( i = 0; i < messungen; ++i )
				akku += messung[i]->kanal(k,t);
			fprintf( output, "%d\t%lf\n", t * 2, akku/messungen );
			}
		fclose( output );
		}

	return EXIT_SUCCESS;
	}