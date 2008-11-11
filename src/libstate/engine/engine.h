/*************************************************
* Engine Header File                             *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_ENGINE_H__
#define BOTAN_ENGINE_H__

#include <botan/scan_name.h>
#include <botan/mutex.h>

#include <botan/block_cipher.h>
#include <botan/stream_cipher.h>
#include <botan/hash.h>
#include <botan/mac.h>

#include <utility>
#include <map>

#if defined(BOTAN_HAS_IF_PUBLIC_KEY_FAMILY)
  #include <botan/if_op.h>
#endif

#if defined(BOTAN_HAS_DSA)
  #include <botan/dsa_op.h>
#endif

#if defined(BOTAN_HAS_DIFFIE_HELLMAN)
  #include <botan/dh_op.h>
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
  #include <botan/nr_op.h>
#endif

#if defined(BOTAN_HAS_ELGAMAL)
  #include <botan/elg_op.h>
#endif

#if defined(BOTAN_HAS_ECDSA)
  #include <botan/ecdsa_op.h>
  #include <botan/ec_dompar.h>
#endif

#if defined(BOTAN_HAS_ECKAEG)
  #include <botan/eckaeg_op.h>
  #include <botan/ec_dompar.h>
#endif

namespace Botan {

class Algorithm_Factory;
class Keyed_Filter;
class Modular_Exponentiator;

/*************************************************
* Engine Base Class                              *
*************************************************/
class BOTAN_DLL Engine
   {
   public:
      template<typename T>
      class BOTAN_DLL Algorithm_Cache
         {
         public:
            virtual T* get(const std::string&) const = 0;
            virtual void add(T* algo, const std::string& = "") const = 0;
            virtual ~Algorithm_Cache() {}
         };

#if defined(BOTAN_HAS_IF_PUBLIC_KEY_FAMILY)
      virtual IF_Operation* if_op(const BigInt&, const BigInt&, const BigInt&,
                                  const BigInt&, const BigInt&, const BigInt&,
                                  const BigInt&, const BigInt&) const
         { return 0; }
#endif

#if defined(BOTAN_HAS_DSA)
      virtual DSA_Operation* dsa_op(const DL_Group&, const BigInt&,
                                    const BigInt&) const
         { return 0; }
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
      virtual NR_Operation* nr_op(const DL_Group&, const BigInt&,
                                  const BigInt&) const
         { return 0; }
#endif

#if defined(BOTAN_HAS_ELGAMAL)
      virtual ELG_Operation* elg_op(const DL_Group&, const BigInt&,
                                    const BigInt&) const
         { return 0; }
#endif

#if defined(BOTAN_HAS_DIFFIE_HELLMAN)
      virtual DH_Operation* dh_op(const DL_Group&, const BigInt&) const
         { return 0; }
#endif

#if defined(BOTAN_HAS_ECDSA)
      virtual ECDSA_Operation* ecdsa_op(const EC_Domain_Params&,
                                        const BigInt&,
                                        const PointGFp&) const
         { return 0; }
#endif

#if defined(BOTAN_HAS_ECKAEG)
      virtual ECKAEG_Operation* eckaeg_op(const EC_Domain_Params&,
                                          const BigInt&,
                                          const PointGFp&) const
         { return 0; }
#endif

      virtual Modular_Exponentiator* mod_exp(const BigInt&,
                                             Power_Mod::Usage_Hints) const
         { return 0; }

      virtual Keyed_Filter* get_cipher(const std::string&, Cipher_Dir)
         { return 0; }

      // Prototype object accessors
      const BlockCipher*
         prototype_block_cipher(const SCAN_Name& request,
                                Algorithm_Factory& af) const;

      const StreamCipher*
         prototype_stream_cipher(const SCAN_Name& request,
                                 Algorithm_Factory& af) const;

      const HashFunction*
         prototype_hash_function(const SCAN_Name& request,
                                 Algorithm_Factory& af) const;

      const MessageAuthenticationCode*
         prototype_mac(const SCAN_Name& request,
                       Algorithm_Factory& af) const;

      // Lookup functions
      virtual BlockCipher* find_block_cipher(const SCAN_Name&,
                                             Algorithm_Factory&) const
         { return 0; }

      virtual StreamCipher* find_stream_cipher(const SCAN_Name&,
                                               Algorithm_Factory&) const
         { return 0; }

      virtual HashFunction* find_hash(const SCAN_Name&,
                                      Algorithm_Factory&) const
         { return 0; }

      virtual MessageAuthenticationCode* find_mac(const SCAN_Name&,
                                                  Algorithm_Factory&) const
         { return 0; }

      // Add new algorithms
      void add_algorithm(BlockCipher*) const;
      void add_algorithm(StreamCipher*) const;
      void add_algorithm(HashFunction*) const;
      void add_algorithm(MessageAuthenticationCode*) const;

      // Engine information
      virtual bool can_add_algorithms() { return false; }
      virtual std::string provider_name() const = 0;

      void initialize(Mutex_Factory& mf);

      Engine();
      virtual ~Engine();

   private:
      Algorithm_Cache<BlockCipher>* cache_of_bc;
      Algorithm_Cache<StreamCipher>* cache_of_sc;
      Algorithm_Cache<HashFunction>* cache_of_hf;
      Algorithm_Cache<MessageAuthenticationCode>* cache_of_mac;
   };

namespace Engine_Core {

/*************************************************
* Get an operation from an Engine                *
*************************************************/
Modular_Exponentiator* mod_exp(const BigInt&, Power_Mod::Usage_Hints);

#if defined(BOTAN_HAS_IF_PUBLIC_KEY_FAMILY)
IF_Operation* if_op(const BigInt&, const BigInt&, const BigInt&,
                    const BigInt&, const BigInt&, const BigInt&,
                    const BigInt&, const BigInt&);
#endif

#if defined(BOTAN_HAS_DSA)
DSA_Operation* dsa_op(const DL_Group&, const BigInt&, const BigInt&);
#endif

#if defined(BOTAN_HAS_NYBERG_RUEPPEL)
NR_Operation* nr_op(const DL_Group&, const BigInt&, const BigInt&);
#endif

#if defined(BOTAN_HAS_ELGAMAL)
ELG_Operation* elg_op(const DL_Group&, const BigInt&, const BigInt&);
#endif

#if defined(BOTAN_HAS_DIFFIE_HELLMAN)
DH_Operation* dh_op(const DL_Group&, const BigInt&);
#endif

#if defined(BOTAN_HAS_ECDSA)
ECDSA_Operation* ecdsa_op(const EC_Domain_Params& dom_pars,
                          const BigInt& priv_key,
                          const PointGFp& pub_key);
#endif

#if defined(BOTAN_HAS_ECKAEG)
ECKAEG_Operation* eckaeg_op(const EC_Domain_Params& dom_pars,
                            const BigInt& priv_key,
                            const PointGFp& pub_key);
#endif

}

}

#endif
