class stats
{
  public:
    void reset();
    void process(double data);

    double average();
    double maxvalue();
    double variance();
    double ops();

  private:
    double sum;
    double sum2;
    double maxval;
    long counter;
};
