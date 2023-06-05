SPHINCS - Fault Injection Attacks
=================================

This work was accepted to [Kangacrypt 2018](https://www.kangacrypt.info/program.php): Aymeric Genêt, Matthias J. Kannwischer, Hervé Pelletier, Andrew McLauchlan, "Practical Fault Injection Attacks on SPHINCS", Kangacrypt 2018


Contents of this repository
===========================
- [code/](code) contains the code that was executed by the target platform (Arduino Due - ARM Cortex M4)
- [attack.py](attack.py) contains the attack code, that can be used to create a forgery from [full_sphincs_signature.txt](full_sphincs_signature.txt) and [wotsp_faulty_sigmas.txt](wotsp_faulty_sigmas.txt) - it makes use of [ChaCha.py](ChaCha.py), [blake.py](blake.py), [bytes_utils.py](bytes_utils.py)

- [full_sphincs_signature.txt](full_sphincs_signature.txt) contains the valid SPHINCS signature obtained from the target device
- [wotsp_valid_sigma.txt](wotsp_valid_sigma.txt) contains the valid W-OTS+ signature obtained from the target device (extracted from [full_sphincs_signature.txt](full_sphincs_signature.txt)) 
- [wotsp_faulty_sigmas.txt](wotsp_faulty_sigmas.txt) contains 85 faulty W-OTS+ signatures obtained from the target device
- [wotsp_sigma_prime.txt](wotsp_sigma_prime.txt) contains the forged W-OTS+ signature (existential forgery)
- [full_sphincs_forged_signature.txt](full_sphincs_forged_signature.txt) containes the forges SPHINCS signature (universal forgery - contains [wotsp_sigma_prime.txt](wotsp_sigma_prime.txt))

- [find_avg_number_of_attempts.py](find_avg_number_of_attempts.py) illustrates how (attack.py)[attack.py] can be used and calculates how many forgery attempts are required if 20 faulty signatures are used 
- [find_success_prob.py](find_success_prob.py) calculates the success probability of a single forgery attempt using 20 faulty signatures. 

Glitch attack
=============

Injecting a glitch in the power supply during the subtree construction results in a faulty signature that contains secret values unintended to be revealed. Exploiting these values results in a universal forgery that we are going to describe here.

SPHINCS-256 full signature
==========================

We ran the standard SPHINCS-256 procedure as described in the original paper (see https://sphincs.cr.yp.to/) with the following message, keys, and masks:

| parameter | value |
----|----
secret key | ```Cé qu'è lainô, le Maitre dé bataille, Que se moqué et se ri dé c ```
masks | ```anaille, A bin fai vi, pè on desande nai, Qu'il étivé patron dé Genevoi. I son vegnu le doze de dessanbro, Pè onna nai asse naire que d'ancro; Y étivé l'an mil si san et dou, Qu'i veniron parla ou pou troi tou. Pè onna nai qu'étive la pe naire, I veniron; y n'étai pas pè bairè: Y étivé pè pilli nou maison, Et no tüa sans aucuna raison Petis et grans, ossis an sevegnance: Pè on matin d'onna bella demanze, Et pè on zeur qu'y fassive bin frai, Sans le bon Di, nos étivon to prai! On vo dera qu'étai cela canaille. - Lou Savoyar contre noutra mouraille Trai eitiellé on dressia et plianta, Et par iqué dou san y son monta. Etian antra, veniron u courdegarda, Yo i firon onna ruda montada. Is avion tenaillé et marté Qu'étivon fai avoi du boun acié, Pè arraci lou cliou et lé saraille, To lou verreu et tota la féraille Qu'on rancontré an dé pari andrai, Et qu'on bouté pè n'eitre pas surprai. On eitablio is avivon forcia; Et d'on petar qu'is avivon teria, I coudavon deiza eitre à sevau: I n'étivon pas assé monta yo. Sen A```

Let a message M = ```Flowers are meant to bloom, not to dry up. That's what I choose to believe.```. The full SPHINCS signature corresponding to this message can be found in ```full_sphincs_signature.txt```.

Target subtree
==============

Let the first subtree of the signature, at address ```07c1d4f115363236```, be the fault attack target. The 67 secret values from the first W-OTS+ signature, which corresponds to the root of the first subtree, can be independently found in ```wotsp_valid_sigma.txt```.

By glitching the Arduino Due, we managed to collect 85 different signatures for this particular signature. They can all be found in ```wotsp_faulty_sigma.txt```.

The python script ```attack.py``` was made to run with SPHINCS-py (https://github.com/joostrijneveld/SPHINCS-py) from Joost Rijneveld. It was however adapted such that it does not require its code.

The forged signature can be found in ```wotsp_sigma_prime.txt```. It is the correct signature for the spoofed message M' = ```aren't there some flowers that never bloom?```.

License
=======
The entire code in this repository is in the [public domain](http://creativecommons.org/publicdomain/zero/1.0/)


References
==========

 * Joost Rijneveld. "SPHINCS-py". GitHub repository. https://github.com/joostrijneveld/SPHINCS-py
