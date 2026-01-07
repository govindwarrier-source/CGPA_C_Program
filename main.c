/*
  mbcet_cgpa_final.c
  Final CGPA/SGPA calculator with CE, ME, EEE, ECE and CS streams (CT removed).
  - Branch order: CE, ME, EEE, ECE, CS
  - P/F subjects: asked by marks, credits = 0 (not counted in GPA). Pass >= 50.
  - Activity points tracked; required >= 150.
  - CS: minors/honours removed (as requested). CE/ME/EEE include minors/honours as per provided syllabus.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BRANCHES 5
#define SEM_COUNT 8
#define MAX_SUBJECTS 10
#define ACTIVITY_REQUIRED 150

typedef struct {
    char name[80];
    int credits;            // 0 for pass/fail (not counted in GPA)
    int marks_obtained;
    int grade_points;       // >=0 for normal mapping; -1 = P/F PASS; -2 = P/F FAIL
    int is_passfail;        // 1 if pass/fail (audit) subject
} Subject;

typedef struct {
    int sem_number;
    Subject subjects[MAX_SUBJECTS];
    int num_subjects;
    float sgpa;
    int is_completed;       // set after marks entered/calculation
    int pf_failed;          // set if any P/F subject failed in this semester
    int activity_points;    // per-semester
} Semester;

typedef struct {
    char name[60];
    char code[6];
    Semester semesters[SEM_COUNT];
    float cgpa;
} Branch;

Branch all_branches[MAX_BRANCHES];

/* Function prototypes */
void initialize_branches();
void preload_subjects_mbcet();
void show_main_menu();
void select_semester(int branch_index);
int get_grade_point(int marks);
void calculate_sgpa_and_cgpa(int branch_index, int sem_index);
void input_marks_and_calculate_sgpa(int branch_index, int sem_index);
void display_report(int branch_index, int sem_index);
int total_activity_points(int branch_index);
int total_pf_failures(int branch_index);
void show_branch_pf_summary(int branch_index);

/* Initialize basic branch metadata and clear arrays */
void initialize_branches() {
    strcpy(all_branches[0].name, "Civil Engineering");                 strcpy(all_branches[0].code, "CE");
    strcpy(all_branches[1].name, "Mechanical Engineering");            strcpy(all_branches[1].code, "ME");
    strcpy(all_branches[2].name, "Electrical & Electronics Engg.");   strcpy(all_branches[2].code, "EEE");
    strcpy(all_branches[3].name, "Electronics & Communication Engg.");strcpy(all_branches[3].code, "ECE");
    strcpy(all_branches[4].name, "Computer Science & Engineering");   strcpy(all_branches[4].code, "CS");

    for (int i = 0; i < MAX_BRANCHES; i++) {
        all_branches[i].cgpa = 0.0f;
        for (int s = 0; s < SEM_COUNT; s++) {
            all_branches[i].semesters[s].sem_number = s + 1;
            all_branches[i].semesters[s].sgpa = 0.0f;
            all_branches[i].semesters[s].is_completed = 0;
            all_branches[i].semesters[s].pf_failed = 0;
            all_branches[i].semesters[s].num_subjects = 0;
            all_branches[i].semesters[s].activity_points = 0;
            for (int k = 0; k < MAX_SUBJECTS; k++) {
                all_branches[i].semesters[s].subjects[k].name[0] = '\0';
                all_branches[i].semesters[s].subjects[k].credits = 0;
                all_branches[i].semesters[s].subjects[k].marks_obtained = -1;
                all_branches[i].semesters[s].subjects[k].grade_points = 0;
                all_branches[i].semesters[s].subjects[k].is_passfail = 0;
            }
        }
    }
}

/* Preload subjects:
   - Exact CE, ME, EEE, ECE and CS syllabi per user-provided lists.
   - P/F subjects: is_passfail = 1, credits = 0 (they won't contribute to GPA)
*/
void preload_subjects_mbcet() {
    /* ===========================
       CE: exact syllabus per user input (index 0)
       =========================== */
    int b_ce = 0;
    // S1 (Total Credits: 20)
    Semester *ce_s1 = &all_branches[b_ce].semesters[0];
    ce_s1->num_subjects = 9;
    strcpy(ce_s1->subjects[0].name, "Linear Algebra and Calculus"); ce_s1->subjects[0].credits = 4; ce_s1->subjects[0].is_passfail = 0;
    strcpy(ce_s1->subjects[1].name, "Engineering Chemistry"); ce_s1->subjects[1].credits = 4; ce_s1->subjects[1].is_passfail = 0;
    strcpy(ce_s1->subjects[2].name, "Engineering Graphics"); ce_s1->subjects[2].credits = 3; ce_s1->subjects[2].is_passfail = 0;
    strcpy(ce_s1->subjects[3].name, "Basics of Electrical Engineering B"); ce_s1->subjects[3].credits = 2; ce_s1->subjects[3].is_passfail = 0;
    strcpy(ce_s1->subjects[4].name, "Basics of Mechanical Engineering"); ce_s1->subjects[4].credits = 2; ce_s1->subjects[4].is_passfail = 0;
    strcpy(ce_s1->subjects[5].name, "Basics of Civil Engineering"); ce_s1->subjects[5].credits = 2; ce_s1->subjects[5].is_passfail = 0;
    strcpy(ce_s1->subjects[6].name, "Environmental Science (P/F)"); ce_s1->subjects[6].credits = 0; ce_s1->subjects[6].is_passfail = 1;
    strcpy(ce_s1->subjects[7].name, "Engineering Chemistry Lab"); ce_s1->subjects[7].credits = 1; ce_s1->subjects[7].is_passfail = 0;
    strcpy(ce_s1->subjects[8].name, "Manufacturing and Construction Practices A"); ce_s1->subjects[8].credits = 1; ce_s1->subjects[8].is_passfail = 0;

    // S2 (Total Credits: 20)
    Semester *ce_s2 = &all_branches[b_ce].semesters[1];
    ce_s2->num_subjects = 8;
    strcpy(ce_s2->subjects[0].name, "Vector Calculus, Differential Equations and Transforms"); ce_s2->subjects[0].credits = 4; ce_s2->subjects[0].is_passfail = 0;
    strcpy(ce_s2->subjects[1].name, "Engineering Physics"); ce_s2->subjects[1].credits = 4; ce_s2->subjects[1].is_passfail = 0;
    strcpy(ce_s2->subjects[2].name, "Applied Mechanics"); ce_s2->subjects[2].credits = 3; ce_s2->subjects[2].is_passfail = 0;
    strcpy(ce_s2->subjects[3].name, "Problem Solving and Programming"); ce_s2->subjects[3].credits = 3; ce_s2->subjects[3].is_passfail = 0;
    strcpy(ce_s2->subjects[4].name, "Building Materials and Construction Technology"); ce_s2->subjects[4].credits = 3; ce_s2->subjects[4].is_passfail = 0;
    strcpy(ce_s2->subjects[5].name, "Professional Communication (P/F)"); ce_s2->subjects[5].credits = 0; ce_s2->subjects[5].is_passfail = 1;
    strcpy(ce_s2->subjects[6].name, "Engineering Physics Lab"); ce_s2->subjects[6].credits = 1; ce_s2->subjects[6].is_passfail = 0;
    strcpy(ce_s2->subjects[7].name, "Design Studio I"); ce_s2->subjects[7].credits = 1; ce_s2->subjects[7].is_passfail = 0;

    // S3 (Total Credits: 22)
    Semester *ce_s3 = &all_branches[b_ce].semesters[2];
    ce_s3->num_subjects = 9;
    strcpy(ce_s3->subjects[0].name, "Partial Differential Equations and Complex Analysis"); ce_s3->subjects[0].credits = 4; ce_s3->subjects[0].is_passfail = 0;
    strcpy(ce_s3->subjects[1].name, "Mechanics of Structures"); ce_s3->subjects[1].credits = 4; ce_s3->subjects[1].is_passfail = 0;
    strcpy(ce_s3->subjects[2].name, "Fluid Mechanics and Hydraulics"); ce_s3->subjects[2].credits = 4; ce_s3->subjects[2].is_passfail = 0;
    strcpy(ce_s3->subjects[3].name, "Surveying and Geomatics"); ce_s3->subjects[3].credits = 3; ce_s3->subjects[3].is_passfail = 0;
    strcpy(ce_s3->subjects[4].name, "Design Engineering"); ce_s3->subjects[4].credits = 2; ce_s3->subjects[4].is_passfail = 0;
    strcpy(ce_s3->subjects[5].name, "Professional Ethics (P/F)"); ce_s3->subjects[5].credits = 0; ce_s3->subjects[5].is_passfail = 1;
    strcpy(ce_s3->subjects[6].name, "Fluid Mechanics Lab"); ce_s3->subjects[6].credits = 2; ce_s3->subjects[6].is_passfail = 0;
    strcpy(ce_s3->subjects[7].name, "Surveying Lab"); ce_s3->subjects[7].credits = 2; ce_s3->subjects[7].is_passfail = 0;
    strcpy(ce_s3->subjects[8].name, "Minor Course"); ce_s3->subjects[8].credits = 3; ce_s3->subjects[8].is_passfail = 0;

    // S4 (Total Credits: 20)
    Semester *ce_s4 = &all_branches[b_ce].semesters[3];
    ce_s4->num_subjects = 8;
    strcpy(ce_s4->subjects[0].name, "Probability, Statistics and Numerical Methods"); ce_s4->subjects[0].credits = 4; ce_s4->subjects[0].is_passfail = 0;
    strcpy(ce_s4->subjects[1].name, "Structural Analysis"); ce_s4->subjects[1].credits = 4; ce_s4->subjects[1].is_passfail = 0;
    strcpy(ce_s4->subjects[2].name, "Hydrology and Water Resources Engineering"); ce_s4->subjects[2].credits = 4; ce_s4->subjects[2].is_passfail = 0;
    strcpy(ce_s4->subjects[3].name, "Water and Wastewater Engineering"); ce_s4->subjects[3].credits = 5; ce_s4->subjects[3].is_passfail = 0;
    strcpy(ce_s4->subjects[4].name, "Universal Human Values II (P/F)"); ce_s4->subjects[4].credits = 0; ce_s4->subjects[4].is_passfail = 1;
    strcpy(ce_s4->subjects[5].name, "Industrial Safety Engineering (P/F)"); ce_s4->subjects[5].credits = 0; ce_s4->subjects[5].is_passfail = 1;
    strcpy(ce_s4->subjects[6].name, "Material Testing Lab I"); ce_s4->subjects[6].credits = 1; ce_s4->subjects[6].is_passfail = 0;
    strcpy(ce_s4->subjects[7].name, "Minor/Honours Course"); ce_s4->subjects[7].credits = 3; ce_s4->subjects[7].is_passfail = 0;

    // S5 (Total Credits: 25)
    Semester *ce_s5 = &all_branches[b_ce].semesters[4];
    ce_s5->num_subjects = 9;
    strcpy(ce_s5->subjects[0].name, "Design of Reinforced Concrete Structures"); ce_s5->subjects[0].credits = 3; ce_s5->subjects[0].is_passfail = 0;
    strcpy(ce_s5->subjects[1].name, "Applications of AI in Civil Engineering"); ce_s5->subjects[1].credits = 3; ce_s5->subjects[1].is_passfail = 0;
    strcpy(ce_s5->subjects[2].name, "Soil Mechanics"); ce_s5->subjects[2].credits = 4; ce_s5->subjects[2].is_passfail = 0;
    strcpy(ce_s5->subjects[3].name, "Transportation Engineering"); ce_s5->subjects[3].credits = 3; ce_s5->subjects[3].is_passfail = 0;
    strcpy(ce_s5->subjects[4].name, "Quantity Surveying and Valuation"); ce_s5->subjects[4].credits = 5; ce_s5->subjects[4].is_passfail = 0;
    strcpy(ce_s5->subjects[5].name, "Program Elective I"); ce_s5->subjects[5].credits = 3; ce_s5->subjects[5].is_passfail = 0;
    strcpy(ce_s5->subjects[6].name, "Material Testing Lab II"); ce_s5->subjects[6].credits = 2; ce_s5->subjects[6].is_passfail = 0;
    strcpy(ce_s5->subjects[7].name, "Transportation Engineering Lab"); ce_s5->subjects[7].credits = 2; ce_s5->subjects[7].is_passfail = 0;
    strcpy(ce_s5->subjects[8].name, "Minor/Honours Course"); ce_s5->subjects[8].credits = 3; ce_s5->subjects[8].is_passfail = 0;

    // S6 (Total Credits: 22)
    Semester *ce_s6 = &all_branches[b_ce].semesters[5];
    ce_s6->num_subjects = 9;
    strcpy(ce_s6->subjects[0].name, "Design of Steel Structures"); ce_s6->subjects[0].credits = 4; ce_s6->subjects[0].is_passfail = 0;
    strcpy(ce_s6->subjects[1].name, "Foundation Engineering"); ce_s6->subjects[1].credits = 3; ce_s6->subjects[1].is_passfail = 0;
    strcpy(ce_s6->subjects[2].name, "Program Elective II"); ce_s6->subjects[2].credits = 3; ce_s6->subjects[2].is_passfail = 0;
    strcpy(ce_s6->subjects[3].name, "Institute Elective I"); ce_s6->subjects[3].credits = 3; ce_s6->subjects[3].is_passfail = 0;
    strcpy(ce_s6->subjects[4].name, "Business Economics and Accountancy"); ce_s6->subjects[4].credits = 3; ce_s6->subjects[4].is_passfail = 0;
    strcpy(ce_s6->subjects[5].name, "Geotechnical Engineering Lab"); ce_s6->subjects[5].credits = 2; ce_s6->subjects[5].is_passfail = 0;
    strcpy(ce_s6->subjects[6].name, "Design Studio II"); ce_s6->subjects[6].credits = 2; ce_s6->subjects[6].is_passfail = 0;
    strcpy(ce_s6->subjects[7].name, "Seminar"); ce_s6->subjects[7].credits = 2; ce_s6->subjects[7].is_passfail = 0;
    strcpy(ce_s6->subjects[8].name, "Minor/Honours Course"); ce_s6->subjects[8].credits = 3; ce_s6->subjects[8].is_passfail = 0;

    // S7 (Total Credits: 21)
    Semester *ce_s7 = &all_branches[b_ce].semesters[6];
    ce_s7->num_subjects = 7;
    strcpy(ce_s7->subjects[0].name, "Traffic Engineering and Management"); ce_s7->subjects[0].credits = 5; ce_s7->subjects[0].is_passfail = 0;
    strcpy(ce_s7->subjects[1].name, "Program Elective III"); ce_s7->subjects[1].credits = 3; ce_s7->subjects[1].is_passfail = 0;
    strcpy(ce_s7->subjects[2].name, "Construction Project Management"); ce_s7->subjects[2].credits = 4; ce_s7->subjects[2].is_passfail = 0;
    strcpy(ce_s7->subjects[3].name, "Institute Elective II"); ce_s7->subjects[3].credits = 3; ce_s7->subjects[3].is_passfail = 0;
    strcpy(ce_s7->subjects[4].name, "Comprehensive Course Viva"); ce_s7->subjects[4].credits = 1; ce_s7->subjects[4].is_passfail = 0;
    strcpy(ce_s7->subjects[5].name, "Project"); ce_s7->subjects[5].credits = 5; ce_s7->subjects[5].is_passfail = 0;
    strcpy(ce_s7->subjects[6].name, "Honours/Minor Course"); ce_s7->subjects[6].credits = 3; ce_s7->subjects[6].is_passfail = 0;

    // S8 (Total Credits: 17)
    Semester *ce_s8 = &all_branches[b_ce].semesters[7];
    ce_s8->num_subjects = 5;
    strcpy(ce_s8->subjects[0].name, "Integrated Waste Management"); ce_s8->subjects[0].credits = 3; ce_s8->subjects[0].is_passfail = 0;
    strcpy(ce_s8->subjects[1].name, "Program Elective IV"); ce_s8->subjects[1].credits = 3; ce_s8->subjects[1].is_passfail = 0;
    strcpy(ce_s8->subjects[2].name, "Program Elective V"); ce_s8->subjects[2].credits = 3; ce_s8->subjects[2].is_passfail = 0;
    strcpy(ce_s8->subjects[3].name, "Program Elective VI"); ce_s8->subjects[3].credits = 3; ce_s8->subjects[3].is_passfail = 0;
    strcpy(ce_s8->subjects[4].name, "Project"); ce_s8->subjects[4].credits = 5; ce_s8->subjects[4].is_passfail = 0;


    /* ===========================
       ME: exact syllabus per user input (index 1)
       =========================== */
    int b_me = 1;
    // S1 (Total Credits: 18)
    Semester *me_s1 = &all_branches[b_me].semesters[0];
    me_s1->num_subjects = 8;
    strcpy(me_s1->subjects[0].name, "Linear Algebra and Calculus"); me_s1->subjects[0].credits = 4; me_s1->subjects[0].is_passfail = 0;
    strcpy(me_s1->subjects[1].name, "Engineering Chemistry"); me_s1->subjects[1].credits = 4; me_s1->subjects[1].is_passfail = 0;
    strcpy(me_s1->subjects[2].name, "Engineering Graphics"); me_s1->subjects[2].credits = 3; me_s1->subjects[2].is_passfail = 0;
    strcpy(me_s1->subjects[3].name, "Basics of Mechanical Engineering"); me_s1->subjects[3].credits = 2; me_s1->subjects[3].is_passfail = 0;
    strcpy(me_s1->subjects[4].name, "Basics of Civil Engineering"); me_s1->subjects[4].credits = 2; me_s1->subjects[4].is_passfail = 0;
    strcpy(me_s1->subjects[5].name, "Environmental Science (P/F)"); me_s1->subjects[5].credits = 0; me_s1->subjects[5].is_passfail = 1;
    strcpy(me_s1->subjects[6].name, "Engineering Chemistry Lab"); me_s1->subjects[6].credits = 1; me_s1->subjects[6].is_passfail = 0;
    strcpy(me_s1->subjects[7].name, "Manufacturing and Construction Practices A"); me_s1->subjects[7].credits = 1; me_s1->subjects[7].is_passfail = 0;

    // S2 (Total Credits: 22)
    Semester *me_s2 = &all_branches[b_me].semesters[1];
    me_s2->num_subjects = 9;
    strcpy(me_s2->subjects[0].name, "Vector Calculus, Differential Equations and Transforms"); me_s2->subjects[0].credits = 4; me_s2->subjects[0].is_passfail = 0;
    strcpy(me_s2->subjects[1].name, "Engineering Physics"); me_s2->subjects[1].credits = 4; me_s2->subjects[1].is_passfail = 0;
    strcpy(me_s2->subjects[2].name, "Engineering Mechanics"); me_s2->subjects[2].credits = 3; me_s2->subjects[2].is_passfail = 0;
    strcpy(me_s2->subjects[3].name, "Problem Solving and Programming in C"); me_s2->subjects[3].credits = 4; me_s2->subjects[3].is_passfail = 0;
    strcpy(me_s2->subjects[4].name, "Basics of Electrical Engineering A"); me_s2->subjects[4].credits = 2; me_s2->subjects[4].is_passfail = 0;
    strcpy(me_s2->subjects[5].name, "Basics of Electronics Engineering"); me_s2->subjects[5].credits = 2; me_s2->subjects[5].is_passfail = 0;
    strcpy(me_s2->subjects[6].name, "Professional Communication (P/F)"); me_s2->subjects[6].credits = 0; me_s2->subjects[6].is_passfail = 1;
    strcpy(me_s2->subjects[7].name, "Engineering Physics Lab"); me_s2->subjects[7].credits = 1; me_s2->subjects[7].is_passfail = 0;
    strcpy(me_s2->subjects[8].name, "Electrical and Electronics Workshop"); me_s2->subjects[8].credits = 1; me_s2->subjects[8].is_passfail = 0;

    // S3 (Total Credits: 22)
    Semester *me_s3 = &all_branches[b_me].semesters[2];
    me_s3->num_subjects = 9;
    strcpy(me_s3->subjects[0].name, "Partial Differential Equations and Complex Analysis"); me_s3->subjects[0].credits = 4; me_s3->subjects[0].is_passfail = 0;
    strcpy(me_s3->subjects[1].name, "Mechanics of Solids"); me_s3->subjects[1].credits = 4; me_s3->subjects[1].is_passfail = 0;
    strcpy(me_s3->subjects[2].name, "Engineering Thermodynamics"); me_s3->subjects[2].credits = 3; me_s3->subjects[2].is_passfail = 0;
    strcpy(me_s3->subjects[3].name, "Metallurgy and Materials Science"); me_s3->subjects[3].credits = 4; me_s3->subjects[3].is_passfail = 0;
    strcpy(me_s3->subjects[4].name, "Mechanics of Fluids"); me_s3->subjects[4].credits = 3; me_s3->subjects[4].is_passfail = 0;
    strcpy(me_s3->subjects[5].name, "Computer Aided Machine Drawing"); me_s3->subjects[5].credits = 2; me_s3->subjects[5].is_passfail = 0;
    strcpy(me_s3->subjects[6].name, "Professional Ethics (P/F)"); me_s3->subjects[6].credits = 0; me_s3->subjects[6].is_passfail = 1;
    strcpy(me_s3->subjects[7].name, "Material Testing Lab"); me_s3->subjects[7].credits = 1; me_s3->subjects[7].is_passfail = 0;
    strcpy(me_s3->subjects[8].name, "Minor Course"); me_s3->subjects[8].credits = 3; me_s3->subjects[8].is_passfail = 0;

    // S4 (Total Credits: 21)
    Semester *me_s4 = &all_branches[b_me].semesters[3];
    me_s4->num_subjects = 9;
    strcpy(me_s4->subjects[0].name, "Probability, Statistics and Numerical Methods"); me_s4->subjects[0].credits = 4; me_s4->subjects[0].is_passfail = 0;
    strcpy(me_s4->subjects[1].name, "Mechanics of Machinery"); me_s4->subjects[1].credits = 4; me_s4->subjects[1].is_passfail = 0;
    strcpy(me_s4->subjects[2].name, "Fluid Machinery"); me_s4->subjects[2].credits = 4; me_s4->subjects[2].is_passfail = 0;
    strcpy(me_s4->subjects[3].name, "Manufacturing Processes"); me_s4->subjects[3].credits = 4; me_s4->subjects[3].is_passfail = 0;
    strcpy(me_s4->subjects[4].name, "Universal Human Values II (P/F)"); me_s4->subjects[4].credits = 0; me_s4->subjects[4].is_passfail = 1;
    strcpy(me_s4->subjects[5].name, "Industrial Safety Engineering (P/F)"); me_s4->subjects[5].credits = 0; me_s4->subjects[5].is_passfail = 1;
    strcpy(me_s4->subjects[6].name, "Hydraulic Machines Lab"); me_s4->subjects[6].credits = 2; me_s4->subjects[6].is_passfail = 0;
    strcpy(me_s4->subjects[7].name, "Machine Tools Lab"); me_s4->subjects[7].credits = 1; me_s4->subjects[7].is_passfail = 0;
    strcpy(me_s4->subjects[8].name, "Minor/Honours Course"); me_s4->subjects[8].credits = 3; me_s4->subjects[8].is_passfail = 0;

    // S5 (Total Credits: 23)
    Semester *me_s5 = &all_branches[b_me].semesters[4];
    me_s5->num_subjects = 9;
    strcpy(me_s5->subjects[0].name, "Dynamics and Design of Machinery"); me_s5->subjects[0].credits = 4; me_s5->subjects[0].is_passfail = 0;
    strcpy(me_s5->subjects[1].name, "Thermal Engineering"); me_s5->subjects[1].credits = 4; me_s5->subjects[1].is_passfail = 0;
    strcpy(me_s5->subjects[2].name, "Machine Tools and Metrology"); me_s5->subjects[2].credits = 4; me_s5->subjects[2].is_passfail = 0;
    strcpy(me_s5->subjects[3].name, "Production and Operations Management"); me_s5->subjects[3].credits = 3; me_s5->subjects[3].is_passfail = 0;
    strcpy(me_s5->subjects[4].name, "Program Elective I (Management Course)"); me_s5->subjects[4].credits = 3; me_s5->subjects[4].is_passfail = 0;
    strcpy(me_s5->subjects[5].name, "Design Engineering"); me_s5->subjects[5].credits = 2; me_s5->subjects[5].is_passfail = 0;
    strcpy(me_s5->subjects[6].name, "Metrology and Instrumentation Lab"); me_s5->subjects[6].credits = 1; me_s5->subjects[6].is_passfail = 0;
    strcpy(me_s5->subjects[7].name, "Thermal Engineering Lab"); me_s5->subjects[7].credits = 2; me_s5->subjects[7].is_passfail = 0;
    strcpy(me_s5->subjects[8].name, "Minor/Honours Course"); me_s5->subjects[8].credits = 3; me_s5->subjects[8].is_passfail = 0;

    // S6 (Total Credits: 22)
    Semester *me_s6 = &all_branches[b_me].semesters[5];
    me_s6->num_subjects = 9;
    strcpy(me_s6->subjects[0].name, "Heat and Mass Transfer"); me_s6->subjects[0].credits = 4; me_s6->subjects[0].is_passfail = 0;
    strcpy(me_s6->subjects[1].name, "Computer Aided Design and FEM"); me_s6->subjects[1].credits = 4; me_s6->subjects[1].is_passfail = 0;
    strcpy(me_s6->subjects[2].name, "Mechatronics and Control Systems"); me_s6->subjects[2].credits = 4; me_s6->subjects[2].is_passfail = 0;
    strcpy(me_s6->subjects[3].name, "Institute Elective I"); me_s6->subjects[3].credits = 3; me_s6->subjects[3].is_passfail = 0;
    strcpy(me_s6->subjects[4].name, "Business Economics and Accountancy"); me_s6->subjects[4].credits = 3; me_s6->subjects[4].is_passfail = 0;
    strcpy(me_s6->subjects[5].name, "Heat Transfer Lab"); me_s6->subjects[5].credits = 2; me_s6->subjects[5].is_passfail = 0;
    strcpy(me_s6->subjects[6].name, "Seminar"); me_s6->subjects[6].credits = 2; me_s6->subjects[6].is_passfail = 0;
    strcpy(me_s6->subjects[7].name, "Minor/Honours Course"); me_s6->subjects[7].credits = 3; me_s6->subjects[7].is_passfail = 0;
    strcpy(me_s6->subjects[8].name, ""); me_s6->subjects[8].credits = 0; me_s6->subjects[8].is_passfail = 0;

    // S7 (Total Credits: 22)
    Semester *me_s7 = &all_branches[b_me].semesters[6];
    me_s7->num_subjects = 8;
    strcpy(me_s7->subjects[0].name, "Design of Machine Elements"); me_s7->subjects[0].credits = 4; me_s7->subjects[0].is_passfail = 0;
    strcpy(me_s7->subjects[1].name, "Robotics and Automation"); me_s7->subjects[1].credits = 4; me_s7->subjects[1].is_passfail = 0;
    strcpy(me_s7->subjects[2].name, "Program Elective II"); me_s7->subjects[2].credits = 3; me_s7->subjects[2].is_passfail = 0;
    strcpy(me_s7->subjects[3].name, "Institute Elective II"); me_s7->subjects[3].credits = 3; me_s7->subjects[3].is_passfail = 0;
    strcpy(me_s7->subjects[4].name, "Mechanical Engineering Lab"); me_s7->subjects[4].credits = 2; me_s7->subjects[4].is_passfail = 0;
    strcpy(me_s7->subjects[5].name, "Comprehensive Viva Voce"); me_s7->subjects[5].credits = 1; me_s7->subjects[5].is_passfail = 0;
    strcpy(me_s7->subjects[6].name, "Project"); me_s7->subjects[6].credits = 5; me_s7->subjects[6].is_passfail = 0;
    strcpy(me_s7->subjects[7].name, "Minor/Honours Course"); me_s7->subjects[7].credits = 3; me_s7->subjects[7].is_passfail = 0;

    // S8 (Total Credits: 17)
    Semester *me_s8 = &all_branches[b_me].semesters[7];
    me_s8->num_subjects = 5;
    strcpy(me_s8->subjects[0].name, "Program Elective III"); me_s8->subjects[0].credits = 3; me_s8->subjects[0].is_passfail = 0;
    strcpy(me_s8->subjects[1].name, "Program Elective IV (Management Course)"); me_s8->subjects[1].credits = 3; me_s8->subjects[1].is_passfail = 0;
    strcpy(me_s8->subjects[2].name, "Program Elective V"); me_s8->subjects[2].credits = 3; me_s8->subjects[2].is_passfail = 0;
    strcpy(me_s8->subjects[3].name, "Program Elective VI"); me_s8->subjects[3].credits = 3; me_s8->subjects[3].is_passfail = 0;
    strcpy(me_s8->subjects[4].name, "Project"); me_s8->subjects[4].credits = 5; me_s8->subjects[4].is_passfail = 0;


    /* ===========================
       EEE: exact syllabus per user input (index 2)
       =========================== */
    int b_eee = 2;
    // S1 (Total Credits: 19)
    Semester *ee_s1 = &all_branches[b_eee].semesters[0];
    ee_s1->num_subjects = 7;
    strcpy(ee_s1->subjects[0].name, "Linear Algebra and Calculus"); ee_s1->subjects[0].credits = 4; ee_s1->subjects[0].is_passfail = 0;
    strcpy(ee_s1->subjects[1].name, "Engineering Chemistry"); ee_s1->subjects[1].credits = 4; ee_s1->subjects[1].is_passfail = 0;
    strcpy(ee_s1->subjects[2].name, "Engineering Graphics"); ee_s1->subjects[2].credits = 3; ee_s1->subjects[2].is_passfail = 0;
    strcpy(ee_s1->subjects[3].name, "Problem Solving and Programming in C"); ee_s1->subjects[3].credits = 4; ee_s1->subjects[3].is_passfail = 0;
    strcpy(ee_s1->subjects[4].name, "Environmental Science (P/F)"); ee_s1->subjects[4].credits = 0; ee_s1->subjects[4].is_passfail = 1;
    strcpy(ee_s1->subjects[5].name, "Engineering Chemistry Lab"); ee_s1->subjects[5].credits = 1; ee_s1->subjects[5].is_passfail = 0;
    strcpy(ee_s1->subjects[6].name, "Manufacturing and Construction Practices B"); ee_s1->subjects[6].credits = 2; ee_s1->subjects[6].is_passfail = 0;

    // S2 (Total Credits: 21)
    Semester *ee_s2 = &all_branches[b_eee].semesters[1];
    ee_s2->num_subjects = 9;
    strcpy(ee_s2->subjects[0].name, "Vector Calculus, Differential Equations and Transforms"); ee_s2->subjects[0].credits = 4; ee_s2->subjects[0].is_passfail = 0;
    strcpy(ee_s2->subjects[1].name, "Engineering Physics"); ee_s2->subjects[1].credits = 4; ee_s2->subjects[1].is_passfail = 0;
    strcpy(ee_s2->subjects[2].name, "Engineering Mechanics"); ee_s2->subjects[2].credits = 3; ee_s2->subjects[2].is_passfail = 0;
    strcpy(ee_s2->subjects[3].name, "Python Programming"); ee_s2->subjects[3].credits = 3; ee_s2->subjects[3].is_passfail = 0;
    strcpy(ee_s2->subjects[4].name, "Basics of Electrical Engineering A"); ee_s2->subjects[4].credits = 2; ee_s2->subjects[4].is_passfail = 0;
    strcpy(ee_s2->subjects[5].name, "Basics of Electronics Engineering"); ee_s2->subjects[5].credits = 2; ee_s2->subjects[5].is_passfail = 0;
    strcpy(ee_s2->subjects[6].name, "Professional Communication (P/F)"); ee_s2->subjects[6].credits = 0; ee_s2->subjects[6].is_passfail = 1;
    strcpy(ee_s2->subjects[7].name, "Engineering Physics Lab"); ee_s2->subjects[7].credits = 1; ee_s2->subjects[7].is_passfail = 0;
    strcpy(ee_s2->subjects[8].name, "Electrical and Electronics Workshop"); ee_s2->subjects[8].credits = 1; ee_s2->subjects[8].is_passfail = 0;

    // S3 (Total Credits: 23)
    Semester *ee_s3 = &all_branches[b_eee].semesters[2];
    ee_s3->num_subjects = 9;
    strcpy(ee_s3->subjects[0].name, "Discrete Mathematical Structures"); ee_s3->subjects[0].credits = 4; ee_s3->subjects[0].is_passfail = 0;
    strcpy(ee_s3->subjects[1].name, "Instrumentation Systems"); ee_s3->subjects[1].credits = 4; ee_s3->subjects[1].is_passfail = 0;
    strcpy(ee_s3->subjects[2].name, "Data Structures"); ee_s3->subjects[2].credits = 4; ee_s3->subjects[2].is_passfail = 0;
    strcpy(ee_s3->subjects[3].name, "Electric Circuit Analysis"); ee_s3->subjects[3].credits = 4; ee_s3->subjects[3].is_passfail = 0;
    strcpy(ee_s3->subjects[4].name, "Design Engineering"); ee_s3->subjects[4].credits = 2; ee_s3->subjects[4].is_passfail = 0;
    strcpy(ee_s3->subjects[5].name, "Professional Ethics (P/F)"); ee_s3->subjects[5].credits = 0; ee_s3->subjects[5].is_passfail = 1;
    strcpy(ee_s3->subjects[6].name, "Data Structures Lab"); ee_s3->subjects[6].credits = 2; ee_s3->subjects[6].is_passfail = 0;
    strcpy(ee_s3->subjects[7].name, "Instrumentation Lab"); ee_s3->subjects[7].credits = 2; ee_s3->subjects[7].is_passfail = 0;
    strcpy(ee_s3->subjects[8].name, "Minor Course"); ee_s3->subjects[8].credits = 3; ee_s3->subjects[8].is_passfail = 0;

    // S4 (Total Credits: 21)
    Semester *ee_s4 = &all_branches[b_eee].semesters[3];
    ee_s4->num_subjects = 8;
    strcpy(ee_s4->subjects[0].name, "Probability, Statistics and Numerical Methods"); ee_s4->subjects[0].credits = 4; ee_s4->subjects[0].is_passfail = 0;
    strcpy(ee_s4->subjects[1].name, "Computer Organization and Architecture"); ee_s4->subjects[1].credits = 4; ee_s4->subjects[1].is_passfail = 0;
    strcpy(ee_s4->subjects[2].name, "Object Oriented Programming using Java"); ee_s4->subjects[2].credits = 5; ee_s4->subjects[2].is_passfail = 0;
    strcpy(ee_s4->subjects[3].name, "Digital Electronics and Logic Design"); ee_s4->subjects[3].credits = 4; ee_s4->subjects[3].is_passfail = 0;
    strcpy(ee_s4->subjects[4].name, "Universal Human Values II (P/F)"); ee_s4->subjects[4].credits = 0; ee_s4->subjects[4].is_passfail = 1;
    strcpy(ee_s4->subjects[5].name, "Industrial Safety Engineering (P/F)"); ee_s4->subjects[5].credits = 0; ee_s4->subjects[5].is_passfail = 1;
    strcpy(ee_s4->subjects[6].name, "Digital Electronics and Logic Design Lab"); ee_s4->subjects[6].credits = 2; ee_s4->subjects[6].is_passfail = 0;
    strcpy(ee_s4->subjects[7].name, "Minor/Honours Course"); ee_s4->subjects[7].credits = 3; ee_s4->subjects[7].is_passfail = 0;

    // S5 (Total Credits: 21)
    Semester *ee_s5 = &all_branches[b_eee].semesters[4];
    ee_s5->num_subjects = 8;
    strcpy(ee_s5->subjects[0].name, "Database Management Systems"); ee_s5->subjects[0].credits = 4; ee_s5->subjects[0].is_passfail = 0;
    strcpy(ee_s5->subjects[1].name, "Microprocessors and Embedded Systems"); ee_s5->subjects[1].credits = 5; ee_s5->subjects[1].is_passfail = 0;
    strcpy(ee_s5->subjects[2].name, "Electrical Machines"); ee_s5->subjects[2].credits = 4; ee_s5->subjects[2].is_passfail = 0;
    strcpy(ee_s5->subjects[3].name, "Business Economics and Accountancy"); ee_s5->subjects[3].credits = 3; ee_s5->subjects[3].is_passfail = 0;
    strcpy(ee_s5->subjects[4].name, "Program Elective I"); ee_s5->subjects[4].credits = 3; ee_s5->subjects[4].is_passfail = 0;
    strcpy(ee_s5->subjects[5].name, "Electrical Machines Lab"); ee_s5->subjects[5].credits = 1; ee_s5->subjects[5].is_passfail = 0;
    strcpy(ee_s5->subjects[6].name, "Database Management Systems Lab"); ee_s5->subjects[6].credits = 1; ee_s5->subjects[6].is_passfail = 0;
    strcpy(ee_s5->subjects[7].name, "Minor/Honours Course"); ee_s5->subjects[7].credits = 3; ee_s5->subjects[7].is_passfail = 0;

    // S6 (Total Credits: 22)
    Semester *ee_s6 = &all_branches[b_eee].semesters[5];
    ee_s6->num_subjects = 9;
    strcpy(ee_s6->subjects[0].name, "Power Electronics"); ee_s6->subjects[0].credits = 4; ee_s6->subjects[0].is_passfail = 0;
    strcpy(ee_s6->subjects[1].name, "Algorithm Analysis and Design"); ee_s6->subjects[1].credits = 4; ee_s6->subjects[1].is_passfail = 0;
    strcpy(ee_s6->subjects[2].name, "Computer Communication and Network Security"); ee_s6->subjects[2].credits = 3; ee_s6->subjects[2].is_passfail = 0;
    strcpy(ee_s6->subjects[3].name, "Program Elective II"); ee_s6->subjects[3].credits = 3; ee_s6->subjects[3].is_passfail = 0;
    strcpy(ee_s6->subjects[4].name, "Institute Elective I"); ee_s6->subjects[4].credits = 3; ee_s6->subjects[4].is_passfail = 0;
    strcpy(ee_s6->subjects[5].name, "Networking Lab"); ee_s6->subjects[5].credits = 1; ee_s6->subjects[5].is_passfail = 0;
    strcpy(ee_s6->subjects[6].name, "Seminar"); ee_s6->subjects[6].credits = 2; ee_s6->subjects[6].is_passfail = 0;
    strcpy(ee_s6->subjects[7].name, "Mini Project"); ee_s6->subjects[7].credits = 2; ee_s6->subjects[7].is_passfail = 0;
    strcpy(ee_s6->subjects[8].name, "Minor/Honours Course"); ee_s6->subjects[8].credits = 3; ee_s6->subjects[8].is_passfail = 0;

    // S7 (Total Credits: 23)
    Semester *ee_s7 = &all_branches[b_eee].semesters[6];
    ee_s7->num_subjects = 8;
    strcpy(ee_s7->subjects[0].name, "Control Systems"); ee_s7->subjects[0].credits = 4; ee_s7->subjects[0].is_passfail = 0;
    strcpy(ee_s7->subjects[1].name, "Power System Engineering"); ee_s7->subjects[1].credits = 4; ee_s7->subjects[1].is_passfail = 0;
    strcpy(ee_s7->subjects[2].name, "Internet of Things"); ee_s7->subjects[2].credits = 3; ee_s7->subjects[2].is_passfail = 0;
    strcpy(ee_s7->subjects[3].name, "Program Elective III"); ee_s7->subjects[3].credits = 3; ee_s7->subjects[3].is_passfail = 0;
    strcpy(ee_s7->subjects[4].name, "Institute Elective II"); ee_s7->subjects[4].credits = 3; ee_s7->subjects[4].is_passfail = 0;
    strcpy(ee_s7->subjects[5].name, "Comprehensive Course Viva"); ee_s7->subjects[5].credits = 1; ee_s7->subjects[5].is_passfail = 0;
    strcpy(ee_s7->subjects[6].name, "Project"); ee_s7->subjects[6].credits = 5; ee_s7->subjects[6].is_passfail = 0;
    strcpy(ee_s7->subjects[7].name, "Minor/Honours Course"); ee_s7->subjects[7].credits = 3; ee_s7->subjects[7].is_passfail = 0;

    // S8 (Total Credits: 17)
    Semester *ee_s8 = &all_branches[b_eee].semesters[7];
    ee_s8->num_subjects = 5;
    strcpy(ee_s8->subjects[0].name, "Program Elective IV"); ee_s8->subjects[0].credits = 3; ee_s8->subjects[0].is_passfail = 0;
    strcpy(ee_s8->subjects[1].name, "Program Elective V"); ee_s8->subjects[1].credits = 3; ee_s8->subjects[1].is_passfail = 0;
    strcpy(ee_s8->subjects[2].name, "Program Elective VI"); ee_s8->subjects[2].credits = 3; ee_s8->subjects[2].is_passfail = 0;
    strcpy(ee_s8->subjects[3].name, "Management for Engineers"); ee_s8->subjects[3].credits = 3; ee_s8->subjects[3].is_passfail = 0;
    strcpy(ee_s8->subjects[4].name, "Project"); ee_s8->subjects[4].credits = 5; ee_s8->subjects[4].is_passfail = 0;


    /* ===========================
       ECE: exact syllabus per user input (index 3)
       (Retained from earlier; P/F flagged)
       =========================== */
    int b_ece = 3;
    // S1 (Total Credits: 19). P/F: Environmental Science (1 credit)
    Semester *e_s1 = &all_branches[b_ece].semesters[0];
    e_s1->num_subjects = 8;
    strcpy(e_s1->subjects[0].name, "Linear Algebra and Calculus"); e_s1->subjects[0].credits = 4; e_s1->subjects[0].is_passfail = 0;
    strcpy(e_s1->subjects[1].name, "Engineering Physics"); e_s1->subjects[1].credits = 4; e_s1->subjects[1].is_passfail = 0;
    strcpy(e_s1->subjects[2].name, "Problem Solving and Programming in C"); e_s1->subjects[2].credits = 4; e_s1->subjects[2].is_passfail = 0;
    strcpy(e_s1->subjects[3].name, "Basics of Electrical Engineering A"); e_s1->subjects[3].credits = 2; e_s1->subjects[3].is_passfail = 0;
    strcpy(e_s1->subjects[4].name, "Basics of Electronics Engineering"); e_s1->subjects[4].credits = 2; e_s1->subjects[4].is_passfail = 0;
    strcpy(e_s1->subjects[5].name, "Environmental Science (P/F)"); e_s1->subjects[5].credits = 0; e_s1->subjects[5].is_passfail = 1;
    strcpy(e_s1->subjects[6].name, "Engineering Physics Lab"); e_s1->subjects[6].credits = 1; e_s1->subjects[6].is_passfail = 0;
    strcpy(e_s1->subjects[7].name, "Electrical and Electronics Workshop"); e_s1->subjects[7].credits = 1; e_s1->subjects[7].is_passfail = 0;

    // S2 (Total Credits: 22). P/F: Professional Communication (1)
    Semester *e_s2 = &all_branches[b_ece].semesters[1];
    e_s2->num_subjects = 8;
    strcpy(e_s2->subjects[0].name, "Vector Calculus, Differential Equations and Transforms"); e_s2->subjects[0].credits = 4; e_s2->subjects[0].is_passfail = 0;
    strcpy(e_s2->subjects[1].name, "Engineering Chemistry"); e_s2->subjects[1].credits = 4; e_s2->subjects[1].is_passfail = 0;
    strcpy(e_s2->subjects[2].name, "Engineering Graphics"); e_s2->subjects[2].credits = 3; e_s2->subjects[2].is_passfail = 0;
    strcpy(e_s2->subjects[3].name, "Python Programming"); e_s2->subjects[3].credits = 3; e_s2->subjects[3].is_passfail = 0;
    strcpy(e_s2->subjects[4].name, "Network Theory"); e_s2->subjects[4].credits = 4; e_s2->subjects[4].is_passfail = 0;
    strcpy(e_s2->subjects[5].name, "Engineering Chemistry Lab"); e_s2->subjects[5].credits = 1; e_s2->subjects[5].is_passfail = 0;
    strcpy(e_s2->subjects[6].name, "Manufacturing and Construction Practices B"); e_s2->subjects[6].credits = 2; e_s2->subjects[6].is_passfail = 0;
    strcpy(e_s2->subjects[7].name, "Professional Communication (P/F)"); e_s2->subjects[7].credits = 0; e_s2->subjects[7].is_passfail = 1;

    // S3 (Total Credits: 23). P/F: Professional Ethics (1)
    Semester *e_s3 = &all_branches[b_ece].semesters[2];
    e_s3->num_subjects = 8;
    strcpy(e_s3->subjects[0].name, "Partial Differential Equation and Complex Analysis"); e_s3->subjects[0].credits = 4; e_s3->subjects[0].is_passfail = 0;
    strcpy(e_s3->subjects[1].name, "Analog Circuits"); e_s3->subjects[1].credits = 4; e_s3->subjects[1].is_passfail = 0;
    strcpy(e_s3->subjects[2].name, "Solid State Devices"); e_s3->subjects[2].credits = 4; e_s3->subjects[2].is_passfail = 0;
    strcpy(e_s3->subjects[3].name, "Logic Circuit Design"); e_s3->subjects[3].credits = 4; e_s3->subjects[3].is_passfail = 0;
    strcpy(e_s3->subjects[4].name, "Design Engineering"); e_s3->subjects[4].credits = 2; e_s3->subjects[4].is_passfail = 0;
    strcpy(e_s3->subjects[5].name, "Professional Ethics (P/F)"); e_s3->subjects[5].credits = 0; e_s3->subjects[5].is_passfail = 1;
    strcpy(e_s3->subjects[6].name, "Analog Circuits Lab"); e_s3->subjects[6].credits = 2; e_s3->subjects[6].is_passfail = 0;
    strcpy(e_s3->subjects[7].name, "Logic Circuit Design Lab"); e_s3->subjects[7].credits = 2; e_s3->subjects[7].is_passfail = 0;

    // S4 (Total Credits: 21). P/F: Universal Human Values II (1), Industrial Safety Engineering (1)
    Semester *e_s4 = &all_branches[b_ece].semesters[3];
    e_s4->num_subjects = 7;
    strcpy(e_s4->subjects[0].name, "Probability, Random Processes and Numerical Methods"); e_s4->subjects[0].credits = 4; e_s4->subjects[0].is_passfail = 0;
    strcpy(e_s4->subjects[1].name, "Linear Integrated Circuits"); e_s4->subjects[1].credits = 4; e_s4->subjects[1].is_passfail = 0;
    strcpy(e_s4->subjects[2].name, "Signals and Systems"); e_s4->subjects[2].credits = 4; e_s4->subjects[2].is_passfail = 0;
    strcpy(e_s4->subjects[3].name, "Microcontroller Based System Design"); e_s4->subjects[3].credits = 5; e_s4->subjects[3].is_passfail = 0;
    strcpy(e_s4->subjects[4].name, "Linear Integrated Circuits Lab"); e_s4->subjects[4].credits = 2; e_s4->subjects[4].is_passfail = 0;
    strcpy(e_s4->subjects[5].name, "Universal Human Values II (P/F)"); e_s4->subjects[5].credits = 0; e_s4->subjects[5].is_passfail = 1;
    strcpy(e_s4->subjects[6].name, "Industrial Safety Engineering (P/F)"); e_s4->subjects[6].credits = 0; e_s4->subjects[6].is_passfail = 1;

    // S5 (Total Credits: 22)
    Semester *e_s5 = &all_branches[b_ece].semesters[4];
    e_s5->num_subjects = 7;
    strcpy(e_s5->subjects[0].name, "Analog and Digital Communication"); e_s5->subjects[0].credits = 4; e_s5->subjects[0].is_passfail = 0;
    strcpy(e_s5->subjects[1].name, "Digital Signal Processing"); e_s5->subjects[1].credits = 4; e_s5->subjects[1].is_passfail = 0;
    strcpy(e_s5->subjects[2].name, "Electromagnetic Field Theory"); e_s5->subjects[2].credits = 4; e_s5->subjects[2].is_passfail = 0;
    strcpy(e_s5->subjects[3].name, "Program Elective I"); e_s5->subjects[3].credits = 3; e_s5->subjects[3].is_passfail = 0;
    strcpy(e_s5->subjects[4].name, "Management for Engineers"); e_s5->subjects[4].credits = 3; e_s5->subjects[4].is_passfail = 0;
    strcpy(e_s5->subjects[5].name, "Communication Lab"); e_s5->subjects[5].credits = 2; e_s5->subjects[5].is_passfail = 0;
    strcpy(e_s5->subjects[6].name, "Digital Signal Processing Lab"); e_s5->subjects[6].credits = 2; e_s5->subjects[6].is_passfail = 0;

    // S6 (Total Credits: 22)
    Semester *e_s6 = &all_branches[b_ece].semesters[5];
    e_s6->num_subjects = 7;
    strcpy(e_s6->subjects[0].name, "Control Systems"); e_s6->subjects[0].credits = 4; e_s6->subjects[0].is_passfail = 0;
    strcpy(e_s6->subjects[1].name, "VLSI Circuit Design"); e_s6->subjects[1].credits = 5; e_s6->subjects[1].is_passfail = 0;
    strcpy(e_s6->subjects[2].name, "Program Elective II"); e_s6->subjects[2].credits = 3; e_s6->subjects[2].is_passfail = 0;
    strcpy(e_s6->subjects[3].name, "Institute Elective I"); e_s6->subjects[3].credits = 3; e_s6->subjects[3].is_passfail = 0;
    strcpy(e_s6->subjects[4].name, "Business Economics and Accountancy"); e_s6->subjects[4].credits = 3; e_s6->subjects[4].is_passfail = 0;
    strcpy(e_s6->subjects[5].name, "Seminar"); e_s6->subjects[5].credits = 2; e_s6->subjects[5].is_passfail = 0;
    strcpy(e_s6->subjects[6].name, "Mini Project"); e_s6->subjects[6].credits = 2; e_s6->subjects[6].is_passfail = 0;

    // S7 (Total Credits: 24)
    Semester *e_s7 = &all_branches[b_ece].semesters[6];
    e_s7->num_subjects = 8;
    strcpy(e_s7->subjects[0].name, "Information Theory and Coding"); e_s7->subjects[0].credits = 4; e_s7->subjects[0].is_passfail = 0;
    strcpy(e_s7->subjects[1].name, "Wireless Communication"); e_s7->subjects[1].credits = 3; e_s7->subjects[1].is_passfail = 0;
    strcpy(e_s7->subjects[2].name, "Computer Networks"); e_s7->subjects[2].credits = 3; e_s7->subjects[2].is_passfail = 0;
    strcpy(e_s7->subjects[3].name, "Program Elective III"); e_s7->subjects[3].credits = 3; e_s7->subjects[3].is_passfail = 0;
    strcpy(e_s7->subjects[4].name, "Institute Elective II"); e_s7->subjects[4].credits = 3; e_s7->subjects[4].is_passfail = 0;
    strcpy(e_s7->subjects[5].name, "Comprehensive Course Viva"); e_s7->subjects[5].credits = 1; e_s7->subjects[5].is_passfail = 0;
    strcpy(e_s7->subjects[6].name, "Project"); e_s7->subjects[6].credits = 5; e_s7->subjects[6].is_passfail = 0;
    strcpy(e_s7->subjects[7].name, "Advanced Communication Lab"); e_s7->subjects[7].credits = 2; e_s7->subjects[7].is_passfail = 0;

    // S8 (Total Credits: 14)
    Semester *e_s8 = &all_branches[b_ece].semesters[7];
    e_s8->num_subjects = 4;
    strcpy(e_s8->subjects[0].name, "Program Elective IV"); e_s8->subjects[0].credits = 3; e_s8->subjects[0].is_passfail = 0;
    strcpy(e_s8->subjects[1].name, "Program Elective V"); e_s8->subjects[1].credits = 3; e_s8->subjects[1].is_passfail = 0;
    strcpy(e_s8->subjects[2].name, "Program Elective VI"); e_s8->subjects[2].credits = 3; e_s8->subjects[2].is_passfail = 0;
    strcpy(e_s8->subjects[3].name, "Project"); e_s8->subjects[3].credits = 5; e_s8->subjects[3].is_passfail = 0;


    /* ===========================
       CS: exact syllabus per user input (index 4), minors/honours removed
       =========================== */
    int b_cs = 4;
    // S1 (Total Credits: 19)
    Semester *c_s1 = &all_branches[b_cs].semesters[0];
    c_s1->num_subjects = 8;
    strcpy(c_s1->subjects[0].name, "Linear Algebra and Calculus"); c_s1->subjects[0].credits = 4; c_s1->subjects[0].is_passfail = 0;
    strcpy(c_s1->subjects[1].name, "Engineering Physics"); c_s1->subjects[1].credits = 4; c_s1->subjects[1].is_passfail = 0;
    strcpy(c_s1->subjects[2].name, "Programming in C"); c_s1->subjects[2].credits = 4; c_s1->subjects[2].is_passfail = 0;
    strcpy(c_s1->subjects[3].name, "Basics of Electrical Engineering A"); c_s1->subjects[3].credits = 2; c_s1->subjects[3].is_passfail = 0;
    strcpy(c_s1->subjects[4].name, "Basics of Electronics Engineering"); c_s1->subjects[4].credits = 2; c_s1->subjects[4].is_passfail = 0;
    strcpy(c_s1->subjects[5].name, "Environmental Science (P/F)"); c_s1->subjects[5].credits = 0; c_s1->subjects[5].is_passfail = 1;
    strcpy(c_s1->subjects[6].name, "Engineering Physics Lab"); c_s1->subjects[6].credits = 1; c_s1->subjects[6].is_passfail = 0;
    strcpy(c_s1->subjects[7].name, "Electrical and Electronics Workshop"); c_s1->subjects[7].credits = 1; c_s1->subjects[7].is_passfail = 0;

    // S2 (Total Credits: 21)
    Semester *c_s2 = &all_branches[b_cs].semesters[1];
    c_s2->num_subjects = 8;
    strcpy(c_s2->subjects[0].name, "Vector Calculus, Differential Equations and Transforms"); c_s2->subjects[0].credits = 4; c_s2->subjects[0].is_passfail = 0;
    strcpy(c_s2->subjects[1].name, "Engineering Chemistry"); c_s2->subjects[1].credits = 4; c_s2->subjects[1].is_passfail = 0;
    strcpy(c_s2->subjects[2].name, "Engineering Graphics"); c_s2->subjects[2].credits = 3; c_s2->subjects[2].is_passfail = 0;
    strcpy(c_s2->subjects[3].name, "Programming using Python"); c_s2->subjects[3].credits = 3; c_s2->subjects[3].is_passfail = 0;
    strcpy(c_s2->subjects[4].name, "Digital Electronics"); c_s2->subjects[4].credits = 3; c_s2->subjects[4].is_passfail = 0;
    strcpy(c_s2->subjects[5].name, "Engineering Chemistry Lab"); c_s2->subjects[5].credits = 1; c_s2->subjects[5].is_passfail = 0;
    strcpy(c_s2->subjects[6].name, "Manufacturing and Construction Practices B"); c_s2->subjects[6].credits = 2; c_s2->subjects[6].is_passfail = 0;
    strcpy(c_s2->subjects[7].name, "Professional Communication (P/F)"); c_s2->subjects[7].credits = 0; c_s2->subjects[7].is_passfail = 1;

    // S3 (Total Credits: 23) — MINOR removed
    Semester *c_s3 = &all_branches[b_cs].semesters[2];
    c_s3->num_subjects = 8;
    strcpy(c_s3->subjects[0].name, "Discrete Mathematical Structures"); c_s3->subjects[0].credits = 4; c_s3->subjects[0].is_passfail = 0;
    strcpy(c_s3->subjects[1].name, "Data Structures"); c_s3->subjects[1].credits = 4; c_s3->subjects[1].is_passfail = 0;
    strcpy(c_s3->subjects[2].name, "Computer Organization and Architecture"); c_s3->subjects[2].credits = 4; c_s3->subjects[2].is_passfail = 0;
    strcpy(c_s3->subjects[3].name, "Object Oriented Programming Concepts"); c_s3->subjects[3].credits = 4; c_s3->subjects[3].is_passfail = 0;
    strcpy(c_s3->subjects[4].name, "Design Engineering"); c_s3->subjects[4].credits = 2; c_s3->subjects[4].is_passfail = 0;
    strcpy(c_s3->subjects[5].name, "Professional Ethics (P/F)"); c_s3->subjects[5].credits = 0; c_s3->subjects[5].is_passfail = 1;
    strcpy(c_s3->subjects[6].name, "Hardware Lab"); c_s3->subjects[6].credits = 2; c_s3->subjects[6].is_passfail = 0;
    strcpy(c_s3->subjects[7].name, "Data Structures Lab"); c_s3->subjects[7].credits = 2; c_s3->subjects[7].is_passfail = 0;

    // S4 (Total Credits: 22) — remove Minor/Honours
    Semester *c_s4 = &all_branches[b_cs].semesters[3];
    c_s4->num_subjects = 8;
    strcpy(c_s4->subjects[0].name, "Probability, Statistics and Numerical Methods"); c_s4->subjects[0].credits = 4; c_s4->subjects[0].is_passfail = 0;
    strcpy(c_s4->subjects[1].name, "Operating Systems"); c_s4->subjects[1].credits = 4; c_s4->subjects[1].is_passfail = 0;
    strcpy(c_s4->subjects[2].name, "Database Management Systems"); c_s4->subjects[2].credits = 4; c_s4->subjects[2].is_passfail = 0;
    strcpy(c_s4->subjects[3].name, "Formal Languages and Automata Theory"); c_s4->subjects[3].credits = 4; c_s4->subjects[3].is_passfail = 0;
    strcpy(c_s4->subjects[4].name, "Universal Human Values II (P/F)"); c_s4->subjects[4].credits = 0; c_s4->subjects[4].is_passfail = 1;
    strcpy(c_s4->subjects[5].name, "Industrial Safety Engineering (P/F)"); c_s4->subjects[5].credits = 0; c_s4->subjects[5].is_passfail = 1;
    strcpy(c_s4->subjects[6].name, "Operating Systems Lab"); c_s4->subjects[6].credits = 2; c_s4->subjects[6].is_passfail = 0;
    strcpy(c_s4->subjects[7].name, "Database Lab"); c_s4->subjects[7].credits = 2; c_s4->subjects[7].is_passfail = 0;

    // S5 (Total Credits: 21) — Minor/Honours removed
    Semester *c_s5 = &all_branches[b_cs].semesters[4];
    c_s5->num_subjects = 7;
    strcpy(c_s5->subjects[0].name, "Computer Networks"); c_s5->subjects[0].credits = 4; c_s5->subjects[0].is_passfail = 0;
    strcpy(c_s5->subjects[1].name, "Microprocessors and Microcontrollers"); c_s5->subjects[1].credits = 4; c_s5->subjects[1].is_passfail = 0;
    strcpy(c_s5->subjects[2].name, "Artificial Intelligence"); c_s5->subjects[2].credits = 3; c_s5->subjects[2].is_passfail = 0;
    strcpy(c_s5->subjects[3].name, "Programme Elective I"); c_s5->subjects[3].credits = 3; c_s5->subjects[3].is_passfail = 0;
    strcpy(c_s5->subjects[4].name, "Business Economics and Accountancy"); c_s5->subjects[4].credits = 3; c_s5->subjects[4].is_passfail = 0;
    strcpy(c_s5->subjects[5].name, "Microprocessor Lab"); c_s5->subjects[5].credits = 2; c_s5->subjects[5].is_passfail = 0;
    strcpy(c_s5->subjects[6].name, "Networking Lab"); c_s5->subjects[6].credits = 2; c_s5->subjects[6].is_passfail = 0;

    // S6 (Total Credits: 26) — Minor/Honours removed
    Semester *c_s6 = &all_branches[b_cs].semesters[5];
    c_s6->num_subjects = 9;
    strcpy(c_s6->subjects[0].name, "Algorithm Analysis and Design"); c_s6->subjects[0].credits = 4; c_s6->subjects[0].is_passfail = 0;
    strcpy(c_s6->subjects[1].name, "Cyber Security"); c_s6->subjects[1].credits = 4; c_s6->subjects[1].is_passfail = 0;
    strcpy(c_s6->subjects[2].name, "Machine Learning"); c_s6->subjects[2].credits = 4; c_s6->subjects[2].is_passfail = 0;
    strcpy(c_s6->subjects[3].name, "Software Engineering Theory and Practices"); c_s6->subjects[3].credits = 4; c_s6->subjects[3].is_passfail = 0;
    strcpy(c_s6->subjects[4].name, "Programme Elective II"); c_s6->subjects[4].credits = 3; c_s6->subjects[4].is_passfail = 0;
    strcpy(c_s6->subjects[5].name, "Institute Elective I"); c_s6->subjects[5].credits = 3; c_s6->subjects[5].is_passfail = 0;
    strcpy(c_s6->subjects[6].name, "Seminar"); c_s6->subjects[6].credits = 2; c_s6->subjects[6].is_passfail = 0;
    strcpy(c_s6->subjects[7].name, "Mini Project"); c_s6->subjects[7].credits = 2; c_s6->subjects[7].is_passfail = 0;
    strcpy(c_s6->subjects[8].name, ""); c_s6->subjects[8].credits = 0; c_s6->subjects[8].is_passfail = 0;

    // S7 (Total Credits: 21) — Minor/Honours removed
    Semester *c_s7 = &all_branches[b_cs].semesters[6];
    c_s7->num_subjects = 7;
    strcpy(c_s7->subjects[0].name, "Compiler Design"); c_s7->subjects[0].credits = 5; c_s7->subjects[0].is_passfail = 0;
    strcpy(c_s7->subjects[1].name, "Web Technology"); c_s7->subjects[1].credits = 4; c_s7->subjects[1].is_passfail = 0;
    strcpy(c_s7->subjects[2].name, "Programme Elective III"); c_s7->subjects[2].credits = 3; c_s7->subjects[2].is_passfail = 0;
    strcpy(c_s7->subjects[3].name, "Institute Elective II"); c_s7->subjects[3].credits = 3; c_s7->subjects[3].is_passfail = 0;
    strcpy(c_s7->subjects[4].name, "Comprehensive Course Viva"); c_s7->subjects[4].credits = 1; c_s7->subjects[4].is_passfail = 0;
    strcpy(c_s7->subjects[5].name, "Project"); c_s7->subjects[5].credits = 5; c_s7->subjects[5].is_passfail = 0;
    strcpy(c_s7->subjects[6].name, ""); c_s7->subjects[6].credits = 0; c_s7->subjects[6].is_passfail = 0;

    // S8 (Total Credits: 14)
    Semester *c_s8 = &all_branches[b_cs].semesters[7];
    c_s8->num_subjects = 4;
    strcpy(c_s8->subjects[0].name, "Programme Elective IV"); c_s8->subjects[0].credits = 3; c_s8->subjects[0].is_passfail = 0;
    strcpy(c_s8->subjects[1].name, "Programme Elective V"); c_s8->subjects[1].credits = 3; c_s8->subjects[1].is_passfail = 0;
    strcpy(c_s8->subjects[2].name, "Programme Elective VI"); c_s8->subjects[2].credits = 3; c_s8->subjects[2].is_passfail = 0;
    strcpy(c_s8->subjects[3].name, "Project"); c_s8->subjects[3].credits = 5; c_s8->subjects[3].is_passfail = 0;
}

/* marks -> grade point mapping */
int get_grade_point(int marks) {
    if (marks >= 90) return 10;
    if (marks >= 85) return 9;
    if (marks >= 75) return 8;
    if (marks >= 65) return 7;
    if (marks >= 55) return 6;
    if (marks >= 50) return 5;
    return 0; // FAIL
}

/* Calculate SGPA and update CGPA */
void calculate_sgpa_and_cgpa(int branch_index, int sem_index) {
    Branch *br = &all_branches[branch_index];
    Semester *cur = &br->semesters[sem_index];

    float total_credit_points_sem = 0.0f;
    int total_credits_sem = 0;

    for (int i = 0; i < cur->num_subjects; i++) {
        Subject *s = &cur->subjects[i];
        if (s->credits > 0) {
            total_credit_points_sem += (float)s->grade_points * s->credits;
            total_credits_sem += s->credits;
        }
    }

    if (total_credits_sem > 0) cur->sgpa = total_credit_points_sem / total_credits_sem;
    else cur->sgpa = 0.0f;

    float cumulative_cp = 0.0f;
    int cumulative_credits = 0;
    for (int s = 0; s < SEM_COUNT; s++) {
        Semester *sem = &br->semesters[s];
        if (sem->is_completed || s == sem_index) {
            int sem_credits = 0;
            for (int j = 0; j < sem->num_subjects; j++) {
                if (sem->subjects[j].credits > 0) sem_credits += sem->subjects[j].credits;
            }
            cumulative_cp += sem->sgpa * sem_credits;
            cumulative_credits += sem_credits;
        }
    }

    if (cumulative_credits > 0) br->cgpa = cumulative_cp / cumulative_credits;
    else br->cgpa = cur->sgpa;

    cur->is_completed = 1;
}

/* Input marks for subjects in a semester (including P/F) */
void input_marks_and_calculate_sgpa(int branch_index, int sem_index) {
    Branch *br = &all_branches[branch_index];
    Semester *cur = &br->semesters[sem_index];

    if (cur->num_subjects == 0) {
        printf("\nNo subject data for semester %d in branch %s.\n", sem_index+1, br->code);
        return;
    }

    printf("\nEntering marks for %s (%s) - Semester %d\n", br->name, br->code, cur->sem_number);
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < cur->num_subjects; i++) {
        Subject *s = &cur->subjects[i];
        int marks = -1;
        printf("Enter marks (0-100) for [%s] (Credits: %d%s): ",
               s->name, s->credits, s->is_passfail ? " | P/F (not in GPA)" : "");
        while (scanf("%d", &marks) != 1 || marks < 0 || marks > 100) {
            printf("Invalid. Enter marks between 0 and 100: ");
            while (getchar() != '\n');
        }
        s->marks_obtained = marks;
        if (s->is_passfail) {
            s->grade_points = (marks >= 50) ? -1 : -2;
        } else {
            s->grade_points = get_grade_point(marks);
        }
    }

    int ap = -1;
    printf("\nEnter Activity Points earned this semester (0-150): ");
    while (scanf("%d", &ap) != 1 || ap < 0 || ap > 150) {
        printf("Invalid. Enter activity points between 0 and 150: ");
        while (getchar() != '\n');
    }
    cur->activity_points = ap;

    cur->pf_failed = 0;
    for (int i = 0; i < cur->num_subjects; i++) {
        Subject *s = &cur->subjects[i];
        if (s->is_passfail && s->grade_points == -2) {
            cur->pf_failed = 1;
            break;
        }
    }

    calculate_sgpa_and_cgpa(branch_index, sem_index);

    display_report(branch_index, sem_index);
}

/* Display semester report */
void display_report(int branch_index, int sem_index) {
    Branch *br = &all_branches[branch_index];
    Semester *cur = &br->semesters[sem_index];

    printf("\n\n============================================================\n");
    printf("   ACADEMIC REPORT: %s (%s) - SEMESTER %d\n", br->name, br->code, cur->sem_number);
    printf("============================================================\n");
    printf("SUBJECT (truncated)           | CR | MARKS |  GP  | NOTES\n");
    printf("------------------------------------------------------------\n");

    int total_credits = 0;
    float total_credit_points = 0.0f;

    for (int i = 0; i < cur->num_subjects; i++) {
        Subject *s = &cur->subjects[i];
        float cpts = 0.0f;
        int display_gp = 0;

        char note[64] = "";

        if (s->is_passfail) {
            if (s->grade_points == -1) {
                display_gp = 0;
                strcpy(note, "P/F: PASS (not in GPA)");
            }
            else if (s->grade_points == -2) {
                display_gp = -2;
                strcpy(note, "P/F: FAIL (reappear)");
            } else {
                strcpy(note, "P/F: pending");
            }
        } else {
            display_gp = s->grade_points;
            cpts = (float)s->grade_points * s->credits;
            total_credits += s->credits;
            total_credit_points += cpts;

            // *** FAIL display fix ***
            if (s->marks_obtained < 50) {
                strcpy(note, "FAIL (needs reappear)");
            }
        }

        printf("%-28.28s | %2d | %5d | %4d | %s\n",
               s->name, s->credits,
               (s->marks_obtained >= 0 ? s->marks_obtained : 0),
               display_gp, note);
    }

    printf("------------------------------------------------------------\n");
    printf("SEMESTER CREDITS (counted for GPA): %d\n", total_credits);
    printf("SEMESTER CREDIT-POINTS (sum): %.2f\n", total_credit_points);
    printf(">> SGPA: %.2f\n", cur->sgpa);
    printf(">> CGPA: %.2f\n", br->cgpa);
    printf(">> Activity Points (this sem): %d\n", cur->activity_points);
    printf(">> Total Activity Points (so far): %d / %d required\n", total_activity_points(branch_index), ACTIVITY_REQUIRED);

    if (cur->pf_failed) {
        printf(">> WARNING: This semester has FAILED P/F subject(s). Clear them to be eligible for degree.\n");
    } else {
        printf(">> P/F Subjects: All passed (or none present) in this semester.\n");
    }

    printf("============================================================\n");
}

int total_activity_points(int branch_index) {
    int sum = 0;
    for (int s = 0; s < SEM_COUNT; s++) sum += all_branches[branch_index].semesters[s].activity_points;
    return sum;
}

int total_pf_failures(int branch_index) {
    int fails = 0;
    Branch *br = &all_branches[branch_index];
    for (int s = 0; s < SEM_COUNT; s++) {
        for (int i = 0; i < br->semesters[s].num_subjects; i++) {
            Subject *sub = &br->semesters[s].subjects[i];
            if (sub->is_passfail && sub->grade_points == -2) fails++;
        }
    }
    return fails;
}

void show_branch_pf_summary(int branch_index) {
    Branch *br = &all_branches[branch_index];
    int fails = total_pf_failures(branch_index);
    printf("\n--- P/F SUMMARY for %s (%s) ---\n", br->name, br->code);
    if (fails == 0) printf("All pass/fail subjects passed so far.\n");
    else printf("There are %d failed pass/fail subject(s). You must clear them to be eligible for the degree.\n", fails);
}

void select_semester(int branch_index) {
    Branch *br = &all_branches[branch_index];
    int choice = 0;
    while (1) {
        printf("\n--- Branch: %s (%s) ---\n", br->name, br->code);
        for (int i = 0; i < SEM_COUNT; i++) {
            printf("  %d. Semester %d %s\n", i+1, br->semesters[i].sem_number,
                   br->semesters[i].is_completed ? "(Completed)" : "");
        }
        printf("  %d. Show total activity points & P/F summary\n", SEM_COUNT + 1);
        printf("  %d. Back to Main Menu\n", SEM_COUNT + 2);
        printf("Choice: ");
        if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); printf("Invalid.\n"); continue; }
        if (choice >= 1 && choice <= SEM_COUNT) input_marks_and_calculate_sgpa(branch_index, choice - 1);
        else if (choice == SEM_COUNT + 1) {
            int total_ap = total_activity_points(branch_index);
            printf("\nTotal Activity Points for %s: %d / %d\n", br->code, total_ap, ACTIVITY_REQUIRED);
            if (total_ap >= ACTIVITY_REQUIRED) printf(">> Activity requirement satisfied.\n");
            else printf(">> Not satisfied. Need %d more.\n", ACTIVITY_REQUIRED - total_ap);
            show_branch_pf_summary(branch_index);
        }
        else if (choice == SEM_COUNT + 2) return;
        else printf("Invalid choice.\n");
    }
}

void show_main_menu() {
    int choice = 0;
    while (1) {
        printf("\n===== MBCET SGPA / CGPA Calculator (Final) =====\n");
        for (int i = 0; i < MAX_BRANCHES; i++) {
            printf("  %d. %s (%s)\n", i+1, all_branches[i].name, all_branches[i].code);
        }
        printf("  %d. Exit\n", MAX_BRANCHES + 1);
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); printf("Invalid.\n"); continue; }
        if (choice >= 1 && choice <= MAX_BRANCHES) select_semester(choice - 1);
        else if (choice == MAX_BRANCHES + 1) { printf("Exiting. Goodbye!\n"); exit(0); }
        else printf("Invalid.\n");
    }
}

int main() {
    initialize_branches();
    preload_subjects_mbcet();
    printf("MBCET CGPA System initialized. CE, ME, EEE, ECE & CS syllabi loaded (CT removed as requested).\n");
    printf("Note: P/F subjects are asked by marks and do not affect SGPA/CGPA. Pass threshold for P/F = 50.\n");
    show_main_menu();
    return 0;
}
