#include <fstream>

#include <dkg/dkg.h>
#include <third_party/json.hpp>

#include <boost/program_options.hpp>

#define EXPAND_AS_STR( x ) __EXPAND_AS_STR__( x )
#define __EXPAND_AS_STR__( x ) #x

static bool g_bVerboseMode = false;

template<class T>
std::string convertToString(T field_elem) {
  mpz_t t;
  mpz_init(t);

  field_elem.as_bigint().to_mpz(t);

  char * tmp = mpz_get_str(NULL, 10, t);
  mpz_clear(t);

  std::string output = tmp;

  return output;
}

void KeyGeneration(const size_t t, const size_t n) {
  signatures::dkg dkg_instance =  signatures::dkg(t, n);

  std::vector<std::vector<libff::alt_bn128_Fr>> polynomial(n);

  for (auto& pol : polynomial) {
    pol = dkg_instance.GeneratePolynomial();
  }

  std::vector<std::vector<libff::alt_bn128_Fr>> secret_key_contribution(n);
  for (size_t i = 0; i < n; ++i) {
    secret_key_contribution[i] = dkg_instance.SecretKeyContribution(polynomial[i]);
  }

  //we will skip here a verification process

  for (size_t i = 0; i < n; ++i) {
    for (size_t j = i; j < n; ++j) {
      secret_key_contribution[i][j] = secret_key_contribution[j][i];
    }
  }
  
  std::vector<libff::alt_bn128_Fr> secret_key(n);
  for (size_t i = 0; i < n; ++i) {
    secret_key[i] = dkg_instance.SecretKeyShareCreate(secret_key_contribution[i]);
  }

  for (size_t i = 0; i < n; ++i) {
    nlohmann::json secretKey;

    secretKey["secret_key"] = convertToString<libff::alt_bn128_Fr>(secret_key[i]);

    std::string strFileName = "secret_key" + std::to_string(i) + ".json";
    std::ofstream out( strFileName.c_str() );
    out << secretKey.dump( 4 ) << "\n";

    if( g_bVerboseMode )
      std::cout
        << strFileName << " file:\n"
        << secretKey.dump( 4 ) << "\n\n";
  }
}

int main(int argc, const char *argv[]) {
  try {
    boost::program_options::options_description desc("Options");
    desc.add_options()
      ("help", "Show this help screen")
      ("version", "Show version number")
      ("t", boost::program_options::value<size_t>(), "Threshold")
      ("n", boost::program_options::value<size_t>(), "Number of participants")
      ("v", "Verbose mode (optional)")
      ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help") || argc <= 1) {
      std::cout
        << "Distributed key generator, version " << EXPAND_AS_STR( BLS_VERSION ) << '\n'
        << "Usage:\n"
        << "   " << argv[0] << " --t <threshold> --n <num_participants> [--v]" << '\n'
        << desc
        << "Output is set of secret_key<j>.json files where 0 <= j < n.\n"
        ;
      return 0;
    }
    if (vm.count("version")) {
      std::cout
        << EXPAND_AS_STR( BLS_VERSION ) << '\n';
      return 0;
    }

    if (vm.count("t") == 0)
      throw std::runtime_error( "--t is missing (see --help)" );
    if (vm.count("n") == 0)
      throw std::runtime_error( "--n is missing (see --help)" );

    if (vm.count("v"))
      g_bVerboseMode = true;

    size_t t = vm["t"].as<size_t>();
    size_t n = vm["n"].as<size_t>();
    if( g_bVerboseMode )
      std::cout
        << "t = " << t << '\n'
        << "n = " << n << '\n'
        << '\n';
    
    KeyGeneration(t, n);
    return 0; // success
  } catch ( std::exception & ex ) {
    std::string strWhat = ex.what();
    if( strWhat.empty() )
      strWhat = "exception without description";
    std::cerr << "exception: " << strWhat << "\n";
  } catch (...) {
    std::cerr << "unknown exception\n";
  }
  return 1;
}
